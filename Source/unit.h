#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

namespace zlth::unit {
  [[nodiscard]] inline float toMag(float value) {
    return std::exp(std::numbers::ln10_v<float> / 20.0f * value);
  }
  [[nodiscard]] inline float toMagFourthRoot(float value) {
    return std::exp(std::numbers::ln10_v<float> / 80.0f * value);
  }
  [[nodiscard]] inline float magToDB(float value) {
    return 20.0f / std::numbers::ln10_v<float> *std::log(value);
  }
  [[nodiscard]] inline float magSqToDB(float value) {
    return 10.0f / std::numbers::ln10_v<float> *std::log(value);
  }
}
