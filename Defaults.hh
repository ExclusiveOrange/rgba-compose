#pragma once

#include "InputSource.hh"

#include <QSize>

namespace Defaults
{
  constexpr const char *saveImageFormat = "png (*.png)";
  constexpr QSize outputSize = {1, 1};
  constexpr const InputSource inputSource = InputSource::Constant;
}
