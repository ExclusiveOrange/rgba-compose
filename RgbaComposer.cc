#include "RgbaComposer.hh"
#include "ui_RgbaComposer.h"

#include <QtWidgets>
#include <QDebug>

#include <functional>
#include <memory>

namespace
{
  struct ChannelUi
  {
    std::function<QString&(void)> fnInputDirectory;
    std::function<const QString&(void)> fnGetImageFilenameFilter;

    QWidget *mainWidget{};
    QGridLayout *grid{};

    struct State
    {
      bool radioConstantSelected = true;
      bool radioImageSelected = false;
      int channelIndex = 0;
      int constantValue = 0;
      QString filename;

      State(){}
    };

    struct
    {
      QRadioButton *radio{};
      QSpinBox *value{};
    } constant;

    struct
    {
      QRadioButton *radio{};
      QComboBox *comboChannel{};
      QCheckBox *checkInvert{};
      QPushButton *buttonFilename{};
      QString filename; // might be empty
    } image;

    static
    std::unique_ptr<ChannelUi>
    create(QString name, QWidget *parent, State state = {})
    {
      // can't use std::make_unique because constructor is private;
      // this way is not exception safe
      // since this is an internal struct I'm not worried
      return std::unique_ptr<ChannelUi>{new ChannelUi(name, parent, state)};
    }

    ~ChannelUi() = default;

    State
    getState()
    const
    {
      State s;

      s.radioConstantSelected = constant.radio->isChecked();
      s.radioImageSelected = image.radio->isChecked();
      s.channelIndex = image.comboChannel->currentIndex();
      s.constantValue = constant.value->value();
      s.filename = image.buttonFilename->text();

      return s;
    }

  private:
    ChannelUi(QString name, QWidget *parent, State state = {})
    {
      // R  ( ) constant |___________|
      //    (*) image    |filename.png|
      //        [ ] invert

      mainWidget = new QWidget(parent);

      {
        grid = new QGridLayout(mainWidget);

        // label
        {
          auto label = new QLabel(name);
          QFont font = label->font();
          font.setPointSize(32);
          label->setFont(font);
          grid->addWidget(label, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
        }

        // constant
        {
          constant.radio = new QRadioButton("constant", mainWidget);
          constant.radio->setChecked(true);
          grid->addWidget(constant.radio, 0, 1);

          constant.value = new QSpinBox(mainWidget);
          constant.value->setRange(0, 255);
          constant.value->setValue(0);
          constant.value->setAlignment(Qt::AlignHCenter);
          grid->addWidget(constant.value, 0, 2);
        }

        // image
        {
          image.radio = new QRadioButton("image", mainWidget);
          grid->addWidget(image.radio, 1, 1);

          image.buttonFilename = new QPushButton("<choose filename>", mainWidget);
          QObject::connect(image.buttonFilename, &QPushButton::clicked, [=](bool){ onButtonFilename(image.buttonFilename); });
          grid->addWidget(image.buttonFilename, 1, 2, 1, 2);

          image.comboChannel = new QComboBox(mainWidget);
          image.comboChannel->addItems({"red", "green", "blue", "alpha"});
          image.comboChannel->setCurrentIndex(0);
          grid->addWidget(image.comboChannel, 2, 2);

          image.checkInvert = new QCheckBox("invert image", mainWidget);
          grid->addWidget(image.checkInvert, 2, 3);
        }
      }
    }

    void
    onButtonFilename(QPushButton *button)
    {
      QString inputDirectory = fnInputDirectory ? fnInputDirectory() : QDir::rootPath();
      QString imageFilenameFilter = fnGetImageFilenameFilter ? fnGetImageFilenameFilter() : "All files (*.*)";

      QString filename = QFileDialog::getOpenFileName(this->mainWidget, "Select an input image", inputDirectory, imageFilenameFilter);
      if (filename.isEmpty())
        return;

      this->image.filename = filename;
      this->image.buttonFilename->setText(filename);
      this->image.radio->setChecked(true);

      if (fnInputDirectory)
        fnInputDirectory() = QFileInfo(filename).absolutePath();
    }
  };
} // namespace

struct RgbaComposer::Private
{
  QString lastInputDir = QDir::rootPath();
  std::unique_ptr<ChannelUi> channelUis[4]; // RGBA
};

RgbaComposer::RgbaComposer(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::RgbaComposer)
  , p{new Private}
{
  ui->setupUi(this);
  setupUi();
}

RgbaComposer::~RgbaComposer()
{
  delete p;
  delete ui;
}

void RgbaComposer::setupUi()
{

  auto wholeWidget = new QWidget(this);
  auto wholeLayout = new QVBoxLayout(wholeWidget);
  setCentralWidget(wholeWidget);

  for (int c = 0; c < 4; ++c)
  {
    auto ui = ChannelUi::create(QString("RGBA"[c]), wholeWidget);
    ui->fnInputDirectory = [this]() -> QString& { return p->lastInputDir; };

    wholeLayout->addWidget(ui->mainWidget);
    p->channelUis[c] = std::move(ui);
  }
}
