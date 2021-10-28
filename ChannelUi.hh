#pragma once

#include <memory>

class QWidget;
class Settings;

struct IChannelUi
{
  virtual ~IChannelUi() {}
  virtual QWidget *getMainWidget() = 0;
};

std::unique_ptr<IChannelUi>
makeChannelUi(int outputChannel, std::shared_ptr<Settings>, QWidget *parent);
