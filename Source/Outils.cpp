/*
  ==============================================================================

    Outils.h
    Created: 11 Mar 2023 5:41:09pm
    Author:  thoma

  ==============================================================================
*/

#include "Outils.hpp"

namespace noi {

namespace Outils {
/// @brief Slow value change of a parameter, slew factor working best between
/// 0.8 - 0.99
/// @param new_value
/// @param old_value
/// @param slew_factor a bigger slew factor means a slower change, must be <1 to
/// keep stability
/// @return

float slewValue(float new_value, float old_value, float slew_factor) {
  return (new_value * (1.0 - slew_factor)) + (old_value * (slew_factor));
}


/// @brief convert milliseconds to samples
/// @param time in seconds
/// @param sample_rate sample / secondes in Hz
/// @return
float convertMsToSample(float time, float sample_rate) {
  float temp = (sample_rate / 1000.f) * time;
  return temp;
}

int mapValueFloatToInt(float inMin, float inMax, float value, int outMin,
                       int outMax) {
  float ratio = ((outMax - outMin) / (inMax - inMin));
  float offset{outMin - (inMin * ratio)};
  int output = static_cast<int>(value * ratio + offset);
  return output;
}

float mapValue(float value, float inMin, float inMax, float outMin,
               float outMax) {
  float ratio{(outMax - outMin) / (inMax - inMin)};
  float offset{outMin - (inMin * ratio)};
  float output{value * ratio + offset};
  return output;
}

float clipValue(float value, float min, float max) {
  if (value > max) value = max;
  if (value < min) value = min;
  return value;
}

float spliter(float target, float state, float diff) {
  if (state < target) {
    state += (target - state) * 2;
  }
  float slope = 0 - (1 / diff);
  state -= target;
  float coef = 1 + slope * state;
  if (coef < 0) {
    return 0;
  } else
    return coef;
}

float linearCrossfade(float dry, float wet, float parameter) {
  return (dry * (1.0 - parameter)) + (wet * parameter);
}

float equalPowerCrossfade(float dry, float wet, float parameter) {
  parameter = 1. - parameter;
  parameter = (parameter - 0.5) * 2;
  float volumes_dry = sqrt(0.5f * (1.f + parameter));
  float volumes_wet = sqrt(0.5f * (1.f - parameter));
  return (dry * volumes_dry) + (wet * volumes_wet);
}

std::array<float, 2> equalPowerCrossfade(std::array<float, 2> dry, std::array<float, 2> wet, float parameter){
  std::array<float, 2> output;
    parameter = 1. - parameter;
    parameter = (parameter - 0.5) * 2;
    float volumes_dry = sqrt(0.5f * (1.f + parameter));
    float volumes_wet = sqrt(0.5f * (1.f - parameter));
  for (int i = 0; i < 2; i++) {
    output[i] = (dry[i] * volumes_dry) + (wet[i] * volumes_wet);
  }
  return output;
}

LFO::LFO(float sampleRate, float frequence) 
: m_sample_rate {sampleRate}
, m_frequence {frequence}
{}

void LFO::phasor() {
  m_status += m_frequence / m_sample_rate;
  if (m_status > 1) m_status-= 1;
}

float TriangleWave::getNextSample() {
  phasor();
  // shaping
  return (std::abs(m_status - 0.5) * 4) - 1;
}

float TriangleWave::getSample() { return (std::abs(m_status - 0.5) * 4) - 1; }

float SawTooth::getNextSample() {
  phasor();
  return (m_status - 0.5) * 2;
}

float SawTooth::getSample() { return (m_status - 0.5) * 2; }

}  // namespace Outils
}  // namespace noi
