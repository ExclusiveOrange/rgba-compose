#include "ChannelUi.hh"

#include "Constants.hh"
#include "Enums.hh"
#include "getInputImageFilenameFilter.hh"
#include "Settings.hh"

#include <QtWidgets>

#include <functional>

namespace
{
  struct ChannelUi : IChannelUi
  {
    std::shared_ptr<Settings> settings;

    int channel;

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

    ChannelUi(int channel, std::shared_ptr<Settings> settings, QWidget *parent)
      : settings{ settings }
      , channel{ channel }
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
        frame->setStyleSheet(QString(".QFrame{Color: %1}").arg(Constants::colorNames[channel]));
        mainWidget = frame;
      }

      {
        grid = new QGridLayout(mainWidget);
        grid->setColumnStretch(0, 0);
        grid->setColumnStretch(1, 0);
        grid->setColumnStretch(3, 1);

        {
          auto label = new QLabel(Constants::channelNames[channel]);
          QFont font = label->font();
          font.setPointSize(Constants::channelNamePointSize);
          label->setFont(font);
          label->setMinimumWidth(Constants::channelNameMinimumWidth);
          grid->addWidget(label, 0, 0, 3, 1, Qt::AlignHCenter | Qt::AlignVCenter);
        }

        {
          constant.radio = new QRadioButton("constant", mainWidget);
          QObject::connect(constant.radio, &QRadioButton::clicked, [=](bool checked){ if (checked) setInputSourceTo(Enums::InputSource::Constant); });
          grid->addWidget(constant.radio, 0, 1);

          constant.value = new QSpinBox(mainWidget);
          constant.value->setRange(0, 255);
          constant.value->setAlignment(Qt::AlignHCenter);
          QObject::connect(constant.value, QOverload<int>::of(&QSpinBox::valueChanged), [=](int v){ onConstantValue(v); });
          grid->addWidget(constant.value, 0, 2);

          grid->addWidget(new QLabel("in [0, 255]"), 0, 3);
        }

        {
          image.radio = new QRadioButton("image", mainWidget);
          QObject::connect(image.radio, &QRadioButton::clicked, [=](bool checked){ if (checked) setInputSourceTo(Enums::InputSource::Image); });
          grid->addWidget(image.radio, 1, 1);

          image.buttonFilename = new QPushButton("<choose filename>", mainWidget);
          QObject::connect(image.buttonFilename, &QPushButton::clicked, [=](bool){ onButtonFilename(); });
          grid->addWidget(image.buttonFilename, 1, 2, 1, 2);

          image.comboChannel = new QComboBox(mainWidget);
          image.comboChannel->addItems({"red", "green", "blue", "alpha"});
          QObject::connect(image.comboChannel, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int i){ onComboChannel(i); });
          grid->addWidget(image.comboChannel, 2, 2);

          image.checkInvert = new QCheckBox("invert image", mainWidget);
          grid->addWidget(image.checkInvert, 2, 3);
        }
      }
    }

    void
    initUi()
    {
      // TODO
    }

    void
    onButtonFilename()
    {
      // try to get a filename from the user
      QString filename = QFileDialog::getOpenFileName(this->mainWidget, "Select an input image", settings->getInputDir(), getInputImageFilenameFilter());
      if (filename.isEmpty())
        return;

      this->image.filename = filename;
      this->image.buttonFilename->setText(QDir::toNativeSeparators(filename));
      this->image.radio->setChecked(true);

      settings->setInputDir(QFileInfo(filename).absolutePath());
    }

    void
    onComboChannel(int channel)
    {
      // TODO
    }

    void
    onConstantValue(int value)
    {
      // TODO
    }

    void
    setInputSourceTo(Enums::InputSource inputSource)
    {
      qDebug() << "setInputSourceTo(" << inputSource << ")";
    }
  };
}

std::unique_ptr<IChannelUi>
makeChannelUi(int channel, std::shared_ptr<Settings> settings, QWidget *parent)
{
  return std::unique_ptr<ChannelUi>{new ChannelUi(channel, settings, parent)};
}
