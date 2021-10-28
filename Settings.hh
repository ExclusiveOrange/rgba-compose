#pragma once

#include "Defaults.hh"
#include "Enums.hh"

#include <QDir>
#include <QSettings>

class Settings
{
  QSettings settings;

  struct Keys
  {
    static constexpr const char
    *filename = "filename",
    *inputDir = "inputDir",
    *outputChannel = "outputChannel",
    *outputDir = "outputDir",
    *outputFormat = "outputFormat",
    *outputSize = "outputSize";

    struct PerOutputChannel
    {
      static constexpr const char
      *inputChannel = "inputChannel",
      *inputSource = "inputSource";
    } perOutputChannel;
  } keys;

public:

  QString
  getPerOutputChannelPrefix(int outputChannel) const
  {
    return QString("%1_%2/").arg(keys.outputChannel).arg(outputChannel);
  }

  int
  getInputChannel(int outputChannel) const
  {
    return settings.value(getPerOutputChannelPrefix(outputChannel) + keys.perOutputChannel.inputChannel, 0).toInt();
  }

  void
  setInputChannel(int outputChannel, int inputChannel)
  {
    settings.setValue(getPerOutputChannelPrefix(outputChannel) + keys.perOutputChannel.inputChannel, inputChannel);
  }

  Enums::InputSource
  getInputSource(int outputChannel) const
  {
    return settings.value(getPerOutputChannelPrefix(outputChannel) + keys.perOutputChannel.inputSource, QVariant::fromValue(Defaults::inputSource)).value<Enums::InputSource>();
  }

  void
  setInputSource(int outputChannel, Enums::InputSource inputSource)
  {
    settings.setValue(getPerOutputChannelPrefix(outputChannel) + keys.perOutputChannel.inputSource, QVariant::fromValue(inputSource));
  }

  QString
  getInputDir() const
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
  getOutputDir() const
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

  QString
  getOutputFormat() const
  {
    return settings.value(keys.outputFormat, Defaults::saveImageFormat).toString();
  }

  void
  setOutputFormat(QString outputFormat)
  {
    settings.setValue(keys.outputFormat, outputFormat);
  }

  QSize
  getOutputSize() const
  {
    return settings.value(keys.outputSize, Defaults::outputSize).toSize();
  }

  void
  setOutputSize(QSize outputSize)
  {
    settings.setValue(keys.outputSize, outputSize);
  }
};
