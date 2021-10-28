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

  struct Settings
  {
    struct Keys
    {
      static constexpr const char
      *filename = "filename",
      *inputDir = "inputDir",
      *outputDir = "outputDir";
    } keys;

    QString
    getInputDir()
    {
      if (settings.contains(keys.inputDir))
        if (QString inputDir = settings.value(keys.inputDir).toString(); QDir(inputDir).exists())
          return inputDir;

      return QDir::rootPath();
    }

    void
    setInputDir(QString inputDir)
    {
      settings.setValue(keys.inputDir, inputDir);
    }

    QString
    getOutputDir()
    {
      if (settings.contains(keys.outputDir))
        if (QString outputDir = settings.value(keys.outputDir).toString(); QDir(outputDir).exists())
          return outputDir;

      return getInputDir();
    }

    void
    setOutputDir(QString outputDir)
    {
      settings.setValue(keys.outputDir, outputDir);
    }

  private:
    QSettings settings;
  };

  struct ChannelUi
  {
    std::shared_ptr<Settings> settings;
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
    create(QString name, QString colorName, std::shared_ptr<Settings> settings, QWidget *parent)
    {
      // can't use std::make_unique because constructor is private;
      // this way is not exception safe
      // since this is an internal struct I'm not worried
      // but for a public struct/class there are safer ways to do this
      return std::unique_ptr<ChannelUi>{new ChannelUi(name, colorName, settings, parent)};
    }

    ~ChannelUi() = default;

    bool isConstant() const
    {
      return constant.radio->isChecked();
    }

    int constantValue() const
    {
      return constant.value->value();
    }

    bool isImage() const
    {
      return image.radio->isChecked();
    }

    QString imageFilename() const
    {
      return image.filename;
    }

  private:
    ChannelUi(QString name, QString colorName, std::shared_ptr<Settings> settings, QWidget *parent)
      : settings{ settings }
      , name{ name }
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
          QObject::connect(image.buttonFilename, &QPushButton::clicked, [=](bool){ onButtonFilename(); });
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
    onButtonFilename()
    {
      QString imageFilenameFilter = fnGetImageFilenameFilter ? fnGetImageFilenameFilter() : "All files (*.*)";

      // try to get a filename from the user
      QString filename = QFileDialog::getOpenFileName(this->mainWidget, "Select an input image", settings->getInputDir(), imageFilenameFilter);
      if (filename.isEmpty())
        return;

      this->image.filename = filename;
      this->image.buttonFilename->setText(QDir::toNativeSeparators(filename));
      this->image.radio->setChecked(true);

      settings->setInputDir(QFileInfo(filename).absolutePath());
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
  std::shared_ptr<Settings> settings = std::make_unique<Settings>();

  QString inputImageFormatFilter = generateInputImageFilenameFilter();
  QString outputImageFormatFilter = generateOutputImageFilenameFilter();

  QString lastOutputFormat;

  std::unique_ptr<ChannelUi> channelUis[4]; // RGBA

  void
  onButtonSave(QWidget *parent)
  {
    if (lastOutputFormat.isEmpty())
      lastOutputFormat = UiConstant::defaultSaveImageFormat;

    QString filename = QFileDialog::getSaveFileName(parent, "Composite image output filename", settings->getOutputDir(), outputImageFormatFilter, &lastOutputFormat);
    if (filename.isEmpty())
      return;

    settings->setOutputDir(QFileInfo(filename).absolutePath());

    parent->setDisabled(true);

    QImage composition = prepareComposition(parent);
    if (!composition.isNull())
    {
      QImageWriter writer(filename);
      if (!writer.write(composition))
        QMessageBox::critical(parent, "Error saving image file", "Couldn't save image to file " + QDir::toNativeSeparators(filename) + "\n\n" + writer.errorString());
    }

    parent->setDisabled(false);
  }

private:
  QImage
  prepareComposition(QWidget *parent)
  {
    std::map<QString, QImage> images;
    std::function<quint8(int x, int y)> readers[4];
    const std::function<quint8(QRgb)> channelExtractors[4]{
      // QRgb: An ARGB quadruplet on the format #AARRGGBB, equivalent to an unsigned int.
      [](QRgb rgb){ return quint8((rgb >> 16) & 255); }, // red
      [](QRgb rgb){ return quint8((rgb >> 8) & 255); }, // green
      [](QRgb rgb){ return quint8(rgb & 255); }, // blue
      [](QRgb rgb){ return quint8((rgb >> 24) & 255); }}; // alpha

    auto getImage = [&](QString filename) -> QImage
    {

      if (auto mapIt = images.find(filename); mapIt != images.end())
        return mapIt->second;

      QImage image;
      QImageReader reader(filename);
      if (!reader.read(&image))
      {
        QMessageBox::critical(parent, "Error reading image file", "Couldn't read image from file " + QDir::toNativeSeparators(filename) + "\n\n" + reader.errorString());
        return {};
      }

      images.emplace(filename, image);

      return image;
    };

    for (int c = 0; c < 4; ++c)
    {
      auto &channelUi = *channelUis[c];

      if (channelUi.isConstant())
        readers[c] = [v=channelUi.constantValue()](int,int) -> quint8 { return v; };
      else
        if (QImage image = getImage(channelUi.imageFilename()); !image.isNull())
          readers[c] = [image, channelExtractor=channelExtractors[c]](int x, int y) -> quint8 { return channelExtractor(image.pixel(x, y)); };
        else
          return {};
    }





    // TODO
    return {};
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

    auto ui = ChannelUi::create(channelNames[c], colorNames[c], p->settings, wholeWidget);

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
  setMinimumHeight(size().height());
  setMaximumHeight(size().height());
}
