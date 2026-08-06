#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

namespace zlth::unit {
  template <float Divisor>
  [[nodiscard]] inline float foobar(float value) {
    return std::exp(std::numbers::ln10_v<float> / Divisor * value);
  }
  template <float Multiplier>
  [[nodiscard]] inline float qux(float value) {
    return Multiplier / std::numbers::ln10_v<float> *std::log(value);
  }
}
