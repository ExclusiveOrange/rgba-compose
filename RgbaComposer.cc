#include "RgbaComposer.hh"
#include "ui_RgbaComposer.h"

#include "ChannelUi.hh"
#include "Destroyer.hh"
#include "generateInputImageFilenameFilter.hh"
#include "generateOutputImageFilenameFilter.hh"
#include "GetImageSizeDialog.hh"
#include "Settings.hh"

#include <QDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QtWidgets>

#include <functional>
#include <memory>
#include <optional>

namespace
{

  enum class ChangeSource { Settings, Widget };

  struct Private
  {
    std::shared_ptr<Settings> settings = std::make_unique<Settings>();

    QString inputImageFormatFilter = generateInputImageFilenameFilter();
    QString outputImageFormatFilter = generateOutputImageFilenameFilter();

    std::unique_ptr<ChannelUi> channelUis[4]; // RGBA

    void
    onButtonSave(QWidget *parent)
    {
      parent->setDisabled(true);
      Destroyer _parent{[=]{ parent->setDisabled(false); }};

      QImage composition = prepareComposition(parent);
      if (composition.isNull())
        return;

      QString outputFormat = settings->getOutputFormat();
      QString filename = QFileDialog::getSaveFileName(parent, "Composite image output filename", settings->getOutputDir(), outputImageFormatFilter, &outputFormat);
      if (filename.isEmpty())
        return;

      settings->setOutputDir(QFileInfo(filename).absolutePath());
      settings->setOutputFormat(outputFormat);

      QImageWriter writer(filename);
      if (!writer.write(composition))
        QMessageBox::critical(parent, "Error saving image file", "Couldn't save image to file " + QDir::toNativeSeparators(filename) + "\n\n" + writer.errorString());
    }

  private:
    QImage
    prepareComposition(QWidget *parent)
    {
      // RGBA is the customary order and what is presented to the user in the UI, while QImage and QRgb expects ARGB.
      // Thus the order in the arrays below is RGBA, but care is taken when extracting channels and creating a QRgb value.

      std::map<QString, QImage> images;
      std::function<quint8(int x, int y)> pixelReaders[4]; // RGBA
      const std::function<quint8(QRgb)> channelExtractors[4]{ // RGBA
        // QRgb: An ARGB quadruplet on the format #AARRGGBB, equivalent to an unsigned int.
        [](QRgb rgb){ return quint8((rgb >> 16) & 255); }, // red
        [](QRgb rgb){ return quint8((rgb >> 8) & 255); }, // green
        [](QRgb rgb){ return quint8(rgb & 255); }, // blue
        [](QRgb rgb){ return quint8((rgb >> 24) & 255); }}; // alpha

      // sort out which inputs go to which outputs
      const int inputChannels[4]{ // index is output channel in RGBA
        settings->getInputChannel(0),
            settings->getInputChannel(1),
            settings->getInputChannel(2),
            settings->getInputChannel(3)};

      std::optional<QSize> imageSize;

      // if image is already loaded then returns it, else loads it into the map then returns it
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

        if (!imageSize)
          imageSize = image.size();
        else
          if (*imageSize != image.size())
          {
            QMessageBox::critical(parent, "Image size mismatch", "The input images must be the same size but are different sizes.");
            return {};
          }

        images.emplace(filename, image);

        return image;
      };

      // prepare reader functions
      for (int c = 0; c < 4; ++c)
      {
        auto &channelUi = *channelUis[c];
        auto inputChannel = inputChannels[c];
        auto channelExtractor = channelExtractors[inputChannel];

        if (channelUi.isConstant())
          pixelReaders[c] = [v=channelUi.constantValue()](int,int) -> quint8 { return v; };
        else
          if (QImage image = getImage(channelUi.imageFilename()); !image.isNull())
            pixelReaders[c] = [image, channelExtractor](int x, int y) -> quint8 { return channelExtractor(image.pixel(x, y)); };
          else
            return {};
      }

      // if any image was loaded then imageSize should be set;
      // otherwise ask the user what size to make the output image
      if (!imageSize && !(imageSize = getImageSizeFromUser(parent)))
        return {};

      // compose new image
      QImage image(*imageSize, QImage::Format_ARGB32);

      for (int y = 0, yn = imageSize->height(); y < yn; ++y)
        for (int x = 0, xn = imageSize->width(); x < xn; ++x)
        {
          quint8 a = pixelReaders[3](x, y);
          quint8 r = pixelReaders[0](x, y);
          quint8 g = pixelReaders[1](x, y);
          quint8 b = pixelReaders[2](x, y);
          QRgb pixel = (a << 24) | (r << 16) | (g << 8) | b;
          image.setPixel(x, y, pixel);
        }

      return image;
    }

    std::optional<QSize>
    getImageSizeFromUser(QWidget *parent)
    {
      auto dialog = std::make_unique<GetImageSizeDialog>("What image size?", "Since no input images were selected, you must tell me what size to make the output image.", settings->getOutputSize(), parent);
      auto maybeSize = dialog->getImageSizeModal();
      if (maybeSize)
        settings->setOutputSize(*maybeSize);
      return maybeSize;
    }
  };
} // namespace

struct RgbaComposer::Private : public ::Private {};

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

  auto mainWidget = new QWidget(this);
  auto mainLayout = new QVBoxLayout(mainWidget);

  setCentralWidget(mainWidget);

  // RGBA input widgets
  for (int c = 0; c < 4; ++c)
  {
    auto ui = ChannelUi::create(c, p->settings, mainWidget);
    ui->fnGetImageFilenameFilter = [this]() -> const QString& { return p->inputImageFormatFilter; };
    mainLayout->addWidget(ui->mainWidget);
    p->channelUis[c] = std::move(ui);
  }

  // save button
  {
    auto saveButton = new QPushButton("Save Composite Image...", mainWidget);

    auto font = saveButton->font();
    font.setPointSize(Constants::saveButtonPointSize);
    saveButton->setFont(font);

    QObject::connect(saveButton, &QPushButton::clicked, [this](bool){ p->onButtonSave(this); });

    mainLayout->addWidget(saveButton);
  }

  adjustSize();
  setMinimumHeight(size().height());
  setMaximumHeight(size().height());
}
