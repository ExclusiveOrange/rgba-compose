#include "RgbaComposer.hh"
#include "ui_RgbaComposer.h"

#include "ChannelUi.hh"
#include "Constants.hh"
#include "Destroyer.hh"
#include "getOutputImageFilenameFilter.hh"
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
  struct Private
  {
    std::shared_ptr<Settings> settings = std::make_unique<Settings>();
    std::unique_ptr<IChannelUi> channelUis[4]; // RGBA

    void
    onButtonSave(QWidget *parent)
    {
      parent->setDisabled(true);
      Destroyer _parent{[=]{ parent->setDisabled(false); }};

      QImage composition = prepareComposition(parent);
      if (composition.isNull())
        return;

      QString outputFormat = settings->getOutputFormat();
      QString filename = QFileDialog::getSaveFileName(parent, "Composite image output filename", settings->getOutputDir(), getOutputImageFilenameFilter(), &outputFormat);
      if (filename.isEmpty())
        return;

      settings->setOutputDir(QFileInfo(filename).absolutePath());
      settings->setOutputFormat(outputFormat);

      QImageWriter writer(filename);
      if (!writer.write(composition))
        QMessageBox::critical(parent, "Error saving image file", "Couldn't save image to file " + QDir::toNativeSeparators(filename) + "\n\n" + writer.errorString());
    }

  private:

    std::optional<QSize>
    getImageSizeFromUser(QWidget *parent)
    {
      auto dialog = std::make_unique<GetImageSizeDialog>("What image size?", "Since no input images were selected, you must tell me what size to make the output image.", settings->getOutputSize(), parent);
      auto maybeSize = dialog->getImageSizeModal();
      if (maybeSize)
        settings->setOutputSize(*maybeSize);
      return maybeSize;
    }

    std::function<quint8(QRgb)>
    getChannelExtractor(int inputChannel)
    {
      // TODO: this could be rewritten to generate a new function based on an array of indices instead of using an array of pre-generated functions
      const std::function<quint8(QRgb)> channelExtractors[4]{ // RGBA
        // QRgb: An ARGB quadruplet on the format #AARRGGBB, equivalent to an unsigned int.
        [](QRgb rgb){ return quint8((rgb >> 16) & 255); }, // red
        [](QRgb rgb){ return quint8((rgb >> 8) & 255); }, // green
        [](QRgb rgb){ return quint8(rgb & 255); }, // blue
        [](QRgb rgb){ return quint8((rgb >> 24) & 255); }}; // alpha

      return channelExtractors[inputChannel];
    }

    std::function<quint8(int x, int y)>
    makePixelReader(int outputChannel, const std::function<QImage(QString filename)> &getImage)
    {
      switch (settings->getInputSource(outputChannel))
      {
        case InputSource::Constant:
          return [v = settings->getInputConstant(outputChannel)](int,int) -> quint8 { return v; };

        case InputSource::Image:
          if (QImage image = getImage(settings->getInputImageFilename(outputChannel)); !image.isNull())
            return
                [image, channelExtractor = getChannelExtractor(settings->getInputChannel(outputChannel))]
                (int x, int y) -> quint8 { return channelExtractor(image.pixel(x, y)); };
      }

      return {};
    }

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
      for (int outputChannel : {0, 1, 2, 3})
        if (auto pixelReader = makePixelReader(outputChannel, getImage))
          pixelReaders[outputChannel] = std::move(pixelReader);
        else
        {
          QMessageBox::critical(parent, "Internal error", "There was a problem with internal method 'makePixelReader'. This might indicate a problem accessing the QSettings store (the registry on Windows).");
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
          QRgb pixel{};
          for(int c : {3, 0, 1, 2}) // ARGB from RGBA
            pixel = (pixel << 8) | pixelReaders[c](x, y);
          image.setPixel(x, y, pixel);
        }

      return image;
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
  for (int outputChannel : {0, 1, 2, 3})
  {
    auto ui = makeChannelUi(outputChannel, p->settings, mainWidget);
    mainLayout->addWidget(ui->getMainWidget());
    p->channelUis[outputChannel] = std::move(ui);
  }

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
