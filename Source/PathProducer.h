#pragma once

#include "unit.h"
#include "zlth_fifo.h"
#include "zlth_simd.h"
#include <vector>

struct SpectrumRenderData {
  std::vector<float> s_fft2;
  std::vector<float> s_fft3;
  std::array<float, 4> s_tmp2 {-100.0f, -100.0f, -100.0f, -100.0f};
  std::array<float, 4> s_tmp3 {-100.0f, -100.0f, -100.0f, -100.0f};
};

class PathProducer {
public:

  PathProducer(std::array<SampleFifo, 2>& leftScsf): fifo(leftScsf) {
    fft1.fill(0.0f);
    tmp1.fill(0.0f);
    fft3.fill(db_init);
    tmp3.fill(db_init);
    for (int i = 0; i < capacity; ++i) {
      auto& data = pathFifo.getBufferAt(i);
      data.s_fft2.assign(FFT_SIZE_HALF, db_init);
      data.s_fft3.assign(FFT_SIZE_HALF, db_init);
    }
  }

  void process(double sampleRate) {
    std::array<std::vector<float>, 2> buffer {};
    while (fifo[0].getNumAvailable() > 0 && fifo[1].getNumAvailable() > 0) {
      if (!fifo[0].pull(buffer[0]) || !fifo[1].pull(buffer[1])) {
        continue;
      }
      const int bufferSize = buffer[0].size();
      if (FFT_SIZE > bufferSize) {
        const int shiftSize = FFT_SIZE - bufferSize;
        std::memmove(fft_.data(), fft_.data() + bufferSize, shiftSize * sizeof(float));
        std::copy(buffer[0].begin(), buffer[0].end(), fft_.begin() + shiftSize);
      }
      else {
        std::copy(buffer[0].begin() + (bufferSize - FFT_SIZE), buffer[0].end(), fft_.begin());
      }
      std::copy(fft_.begin(), fft_.end(), fft0.begin());
      std::fill(fft0.begin() + FFT_SIZE, fft0.end(), 0.0f);
      zlth::simd::mul_inplace({fft0.data(), FFT_SIZE}, 1.0f / static_cast<float>(FFT_SIZE_HALF));
      windowing.multiplyWithWindowingTable(fft0.data(), FFT_SIZE);
      fftJuce.performFrequencyOnlyForwardTransform(fft0.data(), true);
      const float deltaTime = bufferSize / sampleRate;
      const float smooth_fft1 = 1.0f - std::exp(-deltaTime * 30.0f);
      const float smooth_tmp1 = 1.0f - std::exp(-deltaTime * 10.0f);
      const float smooth_fft3 = deltaTime * 15.0f;
      const float smooth_tmp3 = deltaTime * 6.0f;
      zlth::simd::lerp_inplace(fft1, fft0, smooth_fft1);
      zlth::simd::max_inplace(fft1, fft0);
      for (size_t i = 0; i < FFT_SIZE_HALF; ++i) {
        fft2[i] = zlth::unit::qux<20.0f>(std::max(fft1[i], 1e-20f));
      }
      zlth::simd::sub_inplace(fft3, smooth_fft3);
      zlth::simd::max_inplace(fft3, fft2);
      tmp0[0] = zlth::simd::get_abs_max(buffer[0]);
      tmp0[1] = zlth::simd::get_abs_max(buffer[1]);
      zlth::simd::hadamard_butterfly(buffer[0], buffer[1]);
      tmp0[2] = zlth::simd::get_abs_max(buffer[0]);
      tmp0[3] = zlth::simd::get_abs_max(buffer[1]);
      for (int i = 0; i < 4; ++i) {
        tmp1[i] = std::max(tmp0[i], tmp1[i] + smooth_tmp1 * (tmp0[i] - tmp1[i]));
        tmp2[i] = zlth::unit::qux<20.0f>(std::max(tmp1[i], 1e-20f));
        tmp3[i] = std::max(tmp2[i], tmp3[i] - smooth_tmp3);
      }
    }
    if (auto* renderData = pathFifo.getWriteBuffer()) {
      std::copy(fft2.begin(), fft2.end(), renderData->s_fft2.begin());
      std::copy(fft3.begin(), fft3.end(), renderData->s_fft3.begin());
      for (int i = 0; i < 4; ++i) {
        renderData->s_tmp2[i] = tmp2[i];
        renderData->s_tmp3[i] = tmp3[i];
      }
      pathFifo.finishedWrite();
    }
  }

  int getNumPathsAvailable() const {
    return pathFifo.getNumAvailableForReading();
  }

  bool getPath(SpectrumRenderData& path) {
    auto* renderData = pathFifo.getReadBuffer();
    if (renderData == nullptr) {
      return false;
    }
    path.s_fft2 = renderData->s_fft2;
    path.s_fft3 = renderData->s_fft3;
    for (int i = 0; i < 4; ++i) {
      path.s_tmp2[i] = renderData->s_tmp2[i];
      path.s_tmp3[i] = renderData->s_tmp3[i];
    }
    pathFifo.finishedRead();
    return true;
  }

private:
  static constexpr float db_init {-100.0f};
  static constexpr int FFT_ORDER {12};
  static constexpr int FFT_SIZE {1 << FFT_ORDER};
  static constexpr int FFT_SIZE_HALF {FFT_SIZE / 2};
  static constexpr int capacity {32};
  std::array<float, FFT_SIZE> fft_ {};
  std::array<float, FFT_SIZE * 2> fft0 {};
  std::array<float, FFT_SIZE_HALF> fft1 {}, fft2 {}, fft3 {};
  std::array<float, 4> tmp0 {}, tmp1 {}, tmp2 {}, tmp3 {};
  std::array<SampleFifo, 2>& fifo;
  Fifo<SpectrumRenderData, capacity> pathFifo;
  juce::dsp::WindowingFunction<float> windowing {FFT_SIZE, juce::dsp::WindowingFunction<float>::blackmanHarris, true};
  juce::dsp::FFT fftJuce {FFT_ORDER};
};
