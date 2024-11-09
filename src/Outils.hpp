/*
  ==============================================================================

    Outils.h
    Created: 11 Mar 2023 5:41:09pm
    Author:  thoma

  ==============================================================================
*/

#pragma once

#include <cmath>
#include <vector>
#include <array>
const float cheappi{3.14159265359};

namespace noi {

namespace Outils {
/// @brief Slow value change of a parameter, slew factor working best between
/// 0.8 - 0.99
/// @param new_value
/// @param old_value
/// @param slew_factor a bigger slew factor means a slower change, must be <1 to
/// keep stability
/// @return
float slewValue(float new_value, float old_value, float slew_factor);

float convertMsToSample(float time, float sample_rate);

int mapValueFloatToInt(float inMin, float inMax, float value, int outMin,
                       int outMax);

float mapValue(float value, float inMin, float inMax, float outMin,
               float outMax);

float clipValue(float value, float min, float max);

float spliter(float target, float state, float diff);

/// @brief take two signals and return the linear crossfade
/// @param dry signal
/// @param wet signal
/// @param parameter 0 full dry / 1 full wet
/// @return sum of weighted dry and wet
float linearCrossfade(float dry, float wet, float parameter);

/// @brief take two signals and return the equal power crossfade
/// @param dry signal
/// @param wet signal
/// @param parameter 0 full dry / 1 full wet
/// @return Sum of weighted dry and wet
float equalPowerCrossfade(float dry, float wet, float parameter);
std::array<float, 2> equalPowerCrossfade(std::array<float, 2> dry, std::array<float, 2> wet, float parameter);

class LFO {
 public:
  float m_status{};
  float m_sample_rate;
  float m_frequence;

  LFO(float sampleRate, float frequence);
  void phasor();
  void setFrequency(float frequency) { m_frequence = frequency; }
  /// @brief
  /// @param phase between 0 and 1
  void setPhase(float phase) { m_status = phase; }
  float getNextSample() {
    phasor();
    return m_status;
  }
  float getSample() { return m_status; }
};
class TriangleWave : public LFO {
 public:
  using LFO::LFO;
  float getNextSample();
  float getSample();
};

class SawTooth : public LFO {
 public:
  using LFO::LFO;
  float getNextSample();
  float getSample();
};

}  // namespace Outils
}  // namespace noi
