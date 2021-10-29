#include "ChannelUi.hh"

#include "Constants.hh"
#include "getInputImageFilenameFilter.hh"
#include "Settings.hh"

#include <QSignalBlocker>
#include <QtWidgets>

#include <functional>

namespace
{
  enum class From { Settings, Widget };

  struct ChannelUi : IChannelUi
  {
    std::shared_ptr<Settings> settings;

    int outputChannel;

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
      QComboBox *comboInputChannel{};
      QCheckBox *checkInvert{};
      QPushButton *buttonFilename{};
      QString filename; // might be empty
    } image;

    ChannelUi(int outputChannel, std::shared_ptr<Settings> settings, QWidget *parent)
      : settings{ settings }
      , outputChannel{ outputChannel }
    {
      buildUi(parent);
      initUi();
    }

    ~ChannelUi() override = default;

    QWidget *getMainWidget() override
    {
      return mainWidget;
    }

    void
    buildUi(QWidget *parent)
    {
      // R  ( ) constant |___________|
      //    (*) image    |filename.png|
      //        [r,g,b,a]  [ ] invert

      {
        auto frame = new QFrame(parent);
        frame->setFrameShape(QFrame::Box);
        frame->setStyleSheet(QString(".QFrame{Color: %1}").arg(Constants::colorNames[outputChannel]));
        mainWidget = frame;
      }

      {
        grid = new QGridLayout(mainWidget);
        grid->setColumnStretch(0, 0);
        grid->setColumnStretch(1, 0);
        grid->setColumnStretch(3, 1);

        {
          auto label = new QLabel(Constants::channelNames[outputChannel]);
          QFont font = label->font();
          font.setPointSize(Constants::channelNamePointSize);
          label->setFont(font);
          label->setMinimumWidth(Constants::channelNameMinimumWidth);
          grid->addWidget(label, 0, 0, 3, 1, Qt::AlignHCenter | Qt::AlignVCenter);
        }

        {
          constant.radio = new QRadioButton("constant", mainWidget);
          QObject::connect(constant.radio, &QRadioButton::clicked, [=](bool checked){ if (checked) setInputSource(InputSource::Constant); });
          grid->addWidget(constant.radio, 0, 1);

          constant.value = new QSpinBox(mainWidget);
          constant.value->setRange(0, 255);
          constant.value->setAlignment(Qt::AlignHCenter);
          QObject::connect(constant.value, QOverload<int>::of(&QSpinBox::valueChanged), [=](int v){ setInputConstant(v); });
          grid->addWidget(constant.value, 0, 2);

          grid->addWidget(new QLabel("in [0, 255]"), 0, 3);
        }

        {
          image.radio = new QRadioButton("image", mainWidget);
          QObject::connect(image.radio, &QRadioButton::clicked, [=](bool checked){ if (checked) setInputSource(InputSource::Image); });
          grid->addWidget(image.radio, 1, 1);

          image.buttonFilename = new QPushButton(mainWidget);
          QObject::connect(image.buttonFilename, &QPushButton::clicked, [=](bool){ onButtonFilename(); });
          grid->addWidget(image.buttonFilename, 1, 2, 1, 2);

          image.comboInputChannel = new QComboBox(mainWidget);
          image.comboInputChannel->addItems({"red", "green", "blue", "alpha"});
          QObject::connect(image.comboInputChannel, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int i){ setInputChannel(i); });
          grid->addWidget(image.comboInputChannel, 2, 2);

          image.checkInvert = new QCheckBox("invert image", mainWidget);
          QObject::connect(image.checkInvert, &QCheckBox::clicked, [=](bool i){ setInputImageInvert(i); });
          grid->addWidget(image.checkInvert, 2, 3);
        }
      }
    }

    void
    initUi()
    {
      setInputSource(settings->getInputSource(outputChannel), true);
      setInputConstant(settings->getInputConstant(outputChannel), true);
      setInputImageFilename(settings->getInputImageFilename(outputChannel), true);
      setInputChannel(settings->getInputChannel(outputChannel), true);
      setInputImageInvert(settings->getInputImageInvert(outputChannel), true);
    }

    void
    onButtonFilename()
    {
      // try to get a filename from the user
      QString filename = QFileDialog::getOpenFileName(this->mainWidget, "Select an input image", settings->getInputDir(), getInputImageFilenameFilter());
      if (filename.isEmpty())
        return;

      settings->setInputDir(QFileInfo(filename).absolutePath());
      setInputImageFilename(filename);
      setInputSource(InputSource::Image);
    }

    void
    setInputChannel(int inputChannel, bool fromSettings = false)
    {
      if (!fromSettings)
      {
        setInputSource(InputSource::Image);
        settings->setInputChannel(outputChannel, inputChannel);
      }

      QSignalBlocker b(image.comboInputChannel);
      image.comboInputChannel->setCurrentIndex(inputChannel);
    }

    void
    setInputConstant(quint8 value, bool fromSettings = false)
    {
      if (!fromSettings)
      {
        setInputSource(InputSource::Constant);
        settings->setInputConstant(outputChannel, value);
      }

      QSignalBlocker b(constant.value);
      constant.value->setValue(value);
    }

    void
    setInputImageFilename(QString filename, bool fromSettings = false)
    {
      if (!fromSettings)
        settings->setInputImageFilename(outputChannel, filename);

      QString buttonText = filename.isEmpty() ? "<click to select an image file>" : QDir::toNativeSeparators(filename);
      this->image.buttonFilename->setText(buttonText);
    }

    void
    setInputImageInvert(bool invert, bool fromSettings = false)
    {
      if (!fromSettings)
      {
        setInputSource(InputSource::Image);
        settings->setInputImageInvert(outputChannel, invert);
      }

      QSignalBlocker b(image.checkInvert);
      image.checkInvert->setChecked(invert);
    }

    void
    setInputSource(InputSource inputSource, bool fromSettings = false)
    {
      if (!fromSettings)
        settings->setInputSource(outputChannel, inputSource);

      QRadioButton *radio = inputSource == InputSource::Image ? image.radio : constant.radio;
      QSignalBlocker b(radio);
      radio->setChecked(true);
    }
  };
}

std::unique_ptr<IChannelUi>
makeChannelUi(int outputChannel, std::shared_ptr<Settings> settings, QWidget *parent)
{
  return std::unique_ptr<ChannelUi>{new ChannelUi(outputChannel, settings, parent)};
}
