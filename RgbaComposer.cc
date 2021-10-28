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
    static constexpr const char *defaultSaveImageFormat = "png (*.png)";
  }

  namespace SettingsKey
  {
    static constexpr const char *filename = "filename";
  }

  struct ChannelUi
  {
    std::function<QString&(void)> fnInputDirectory;
    std::function<const QString&(void)> fnGetImageFilenameFilter;

    QString name;
    QWidget *mainWidget{};
    QGridLayout *grid{};

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
    create(QString name, QString colorName, QWidget *parent)
    {
      // can't use std::make_unique because constructor is private;
      // this way is not exception safe
      // since this is an internal struct I'm not worried
      // but for a public struct/class there are safer ways to do this
      return std::unique_ptr<ChannelUi>{new ChannelUi(name, colorName, parent)};
    }

    ~ChannelUi() = default;

  private:
    ChannelUi(QString name, QString colorName, QWidget *parent)
      : name{ name }
    {
      // R  ( ) constant |___________|
      //    (*) image    |filename.png|
      //        [r,g,b,a]  [ ] invert

      {
        auto frame = new QFrame(parent);
        frame->setFrameShape(QFrame::Box);
        frame->setStyleSheet(QString(".QFrame{Color: %1}").arg(colorName));
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
  generateInputImageFilenameFilter()
  {
    QStringList formats;
    for (const QByteArray &format : QImageReader::supportedImageFormats())
      formats.append(format);

    QStringList formatFilters;
    for (const QString &format : formats)
      formatFilters.append("*." + format);

    QString allFilter = "Images (" + formatFilters.join(" ") + ")";

    QStringList filter(allFilter);
    for (const QString &format : formats)
      filter.append(QString("%1 (*.%1)").arg(format));

    return filter.join(";;");
  }

  QString
  generateOutputImageFilenameFilter()
  {
    QStringList formats;
    for (const QByteArray &format : QImageWriter::supportedImageFormats())
      formats.append(format);

    QStringList filter;
    for (const QString &format : formats)
      filter.append(QString("%1 (*.%1)").arg(format));

    return filter.join(";;");
  }
} // namespace

struct RgbaComposer::Private
{
  QString inputImageFormatFilter = generateInputImageFilenameFilter();
  QString lastInputDir = QDir::rootPath();

  QString outputImageFormatFilter = generateOutputImageFilenameFilter();
  QString lastOutputDir;
  QString lastOutputFormat;

  std::unique_ptr<ChannelUi> channelUis[4]; // RGBA


  void
  onButtonSave(QWidget *parent)
  {
    if (lastOutputDir.isEmpty())
      lastOutputDir = lastInputDir; // lastInputDir should have a valid default

    if (lastOutputFormat.isEmpty())
      lastOutputFormat = UiConstant::defaultSaveImageFormat;

    QString filename = QFileDialog::getSaveFileName(parent, "Composite image output filename", lastOutputDir, outputImageFormatFilter, &lastOutputFormat);
    if (filename.isEmpty())
      return;

    lastOutputDir = QFileInfo(filename).absolutePath();

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
    constexpr const char *channelNames[4] = {"R", "G", "B", "A"};
    constexpr const char *colorNames[4] = {"Red", "Green", "Blue", "Black"};

    auto ui = ChannelUi::create(channelNames[c], colorNames[c], wholeWidget);

    ui->fnInputDirectory = [this]() -> QString& { return p->lastInputDir; };
    ui->fnGetImageFilenameFilter = [this]() -> const QString& { return p->inputImageFormatFilter; };

    wholeLayout->addWidget(ui->mainWidget);

    p->channelUis[c] = std::move(ui);
  }

  // save button
  {
    auto saveButton = new QPushButton("Save Composite Image...", wholeWidget);

    auto font = saveButton->font();
    font.setPointSize(UiConstant::saveButtonPointSize);
    saveButton->setFont(font);

    QObject::connect(saveButton, &QPushButton::clicked, [this](bool){ p->onButtonSave(this); });

    wholeLayout->addWidget(saveButton);
  }

  adjustSize();
}
