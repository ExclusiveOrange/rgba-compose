#pragma once

#include "Enums.hh"

#include <QSize>

namespace Defaults
{
  constexpr const char *saveImageFormat = "png (*.png)";
  constexpr QSize outputSize = {1, 1};
  constexpr const Enums::InputSource inputSource = Enums::InputSource::Constant;
}
