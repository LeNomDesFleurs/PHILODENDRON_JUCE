#pragma once

#include <JuceHeader.h>

#include <iostream>
#include <vector>
#include <array>

#include "Outils.hpp"

//in samples
static const int CROSSFADE_SIZE = 10000;
static const int NUMBER_OF_HEADS = 4;

namespace noi {

//custom version for Pilodendron
//The read reference keep de position of the correct delay
//if the read speed is different from 0
//loop between the write and read_reference pointer
//if freezed, loop in the whole buffer
class RingBuffer {
 public:
  RingBuffer(float max_time, float initial_delay, int _sample_rate);
  void reset(float max_time, float initial_delay, int _sample_rate);
  float readSample();
  void writeSample(float input_sample);
  float interpolate();
  float linearInterpolation();
  float noInterpolation();
  float allpassInterpolation();
  void incrementReadPointerReference();
  void freezeIncrementReadPointerReference();
  void setStepSize(float step_size);
  void setDelayTime(float delay_time);
  void updateStepSize();
  void freezedUpdateStepSize();
  void setSampleRate(float _sample_rate);
  void setFreezed(bool _freezed);
  void checkForReadIndexOverFlow();
  void setHeadsReadSpeed(float base_read_speed, float ratio);
  /// @brief Split read index in m_i_read (previous sample) m_i_read_next (next
  /// sample) and m_frac (fractional offset)
  void fractionalizeReadIndex();
  float getActualSize();
  void crossfade();
  /// offset between 0 and 1
  void setReadOffset(float offset);

class Head{
  public:
  /// @brief distance to read reference
  float distance{};
  float read_speed{};

void increment(int max_distance){

    distance += read_speed;

    if (distance > max_distance) {
    distance = 0;
  }

  if (distance < 0){
    distance = max_distance;
  }
}
};

std::array<Head, 4> heads;


// std::array<int, 4> heads_outputs;
int active_heads;
// the accumulating status is used before freezing,
// it collect some more sample before making a fade with the previous signal,
// a simpe freeze whitout this fade would produce clicks when going from last
// wrote to first wrote
int sample_rate;
enum BufferMode { normal, accumulate, freeze, reverse, repitch };
BufferMode m_buffer_mode{normal};
enum InterpolationMode { none, linear, allpass };
InterpolationMode interpolation_mode = linear;
std::vector<float> m_buffer;
std::vector<float> m_crossfade_buffer;
int m_write;

bool crossfading{};

float m_read_reference{};
float m_read_offset{};
float distance{};
float read_speed{};
float crossfade_read_head{};

// index used to retrieve samples with the interpolation functions
double m_read{};
int m_i_read{};
int m_i_read_next{};
float m_frac{};

/// step size used when modifying the size of the buffer, allowing resizing
/// without clicks (generating repitch)
float m_step_size{1.0};

int m_size_goal;
// m_buffer_size en base 0
int m_buffer_size;
float m_actual_size{};
float m_size_on_freeze{};
float m_output_sample{};
float m_distance_to_cover{};
bool new_size{false};
bool ready_to_lock{false};
// number of sample accumulated before fading and freezing
int accumulate_count{};
};
}  // namespace noi
