#include "RgbaComposer.hh"
#include "ui_RgbaComposer.h"

#include <QDebug>
#include <QImageReader>
#include <QtWidgets>

#include <functional>
#include <memory>

// TODO: use QSettings to remember user choices

namespace
{
  namespace UiConstant
  {
    static constexpr int channelNamePointSize = 32;
    static constexpr int channelNameMinimumWidth = 50;
    static constexpr int saveButtonPointSize = 24;
  }

  struct ChannelUi
  {
    std::function<QString&(void)> fnInputDirectory;
    std::function<const QString&(void)> fnGetImageFilenameFilter;

    QString name;
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
      // but for a public struct/class there are safer ways to do this
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
      : name{ name }
    {
      // R  ( ) constant |___________|
      //    (*) image    |filename.png|
      //        [r,g,b,a]  [ ] invert

//      mainWidget = new QWidget(parent);
      {
        auto frame = new QFrame(parent);
        frame->setFrameShape(QFrame::Box);
        mainWidget = frame;
      }

      {
        grid = new QGridLayout(mainWidget);
        grid->setColumnStretch(0, 0);
        grid->setColumnStretch(1, 0);
        grid->setColumnStretch(3, 1);

        // label
        {
          auto label = new QLabel(name);
          QFont font = label->font();
          font.setPointSize(UiConstant::channelNamePointSize);
          label->setFont(font);
          label->setMinimumWidth(UiConstant::channelNameMinimumWidth);
          grid->addWidget(label, 0, 0, 3, 1, Qt::AlignHCenter | Qt::AlignVCenter);
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
          QObject::connect(constant.value, QOverload<int>::of(&QSpinBox::valueChanged), [=](int){ constant.radio->setChecked(true); });
          grid->addWidget(constant.value, 0, 2);

          grid->addWidget(new QLabel("in [0, 255]"), 0, 3);
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

      // try to get a filename from the user
      QString filename = QFileDialog::getOpenFileName(this->mainWidget, "Select an input image", inputDirectory, imageFilenameFilter);
      if (filename.isEmpty())
        return;

      this->image.filename = filename;
      this->image.buttonFilename->setText(QDir::toNativeSeparators(filename));
      this->image.radio->setChecked(true);

      // remember this directory for the next time an open file dialog is shown
      if (fnInputDirectory)
        fnInputDirectory() = QFileInfo(filename).absolutePath();
    }
  };

  QString
  generateImageFilenameFilter()
  {
    QStringList extensions;
    for (const QByteArray &extension : QImageReader::supportedImageFormats())
      extensions.append(extension);

    QStringList extensionsFilters;
    for (const QString &extension : extensions)
      extensionsFilters.append("*." + extension);

    QString all = "Images (" + extensionsFilters.join(" ") + ")";

    QStringList formats(all);
    for (const QString &extension : extensions)
      formats.append(QString("%1 (*.%1)").arg(extension));

    return formats.join(";;");
  }
} // namespace

struct RgbaComposer::Private
{
  QString lastInputDir = QDir::rootPath();
  QString imageFilenameFilter = generateImageFilenameFilter();
  std::unique_ptr<ChannelUi> channelUis[4]; // RGBA

  void
  onButtonSave()
  {
    // TODO: get filename or cancel
    // TODO: show "please wait" or something
    // TODO: get or create four single-color channel images
    // TODO: combine channel images into single image
    // TODO: write single image to file
    // TODO: show "done" message"
  }
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

  // RGBA input widgets
  for (int c = 0; c < 4; ++c)
  {
    auto ui = ChannelUi::create(QString("RGBA"[c]), wholeWidget);
    ui->fnInputDirectory = [this]() -> QString& { return p->lastInputDir; };
    ui->fnGetImageFilenameFilter = [this]() -> const QString& { return p->imageFilenameFilter; };
    wholeLayout->addWidget(ui->mainWidget);
    p->channelUis[c] = std::move(ui);
  }

  // save button
  {
    auto saveButton = new QPushButton("Save Composite Image...", wholeWidget);
    auto font = saveButton->font();
    font.setPointSize(UiConstant::saveButtonPointSize);
    saveButton->setFont(font);
    QObject::connect(saveButton, &QPushButton::clicked, [this](bool){ p->onButtonSave(); });
    wholeLayout->addWidget(saveButton);
  }

  adjustSize();
}
