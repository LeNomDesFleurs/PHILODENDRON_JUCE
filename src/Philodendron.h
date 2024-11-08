/*
  ==============================================================================

    Hellebore.h
    Created: 11 Mar 2023 2:28:59pm
    Author:  thoma

  ==============================================================================
*/

#pragma once

#include <array>
#include <vector>

// #include "Filter.hpp"
#include "RingBuffer.hpp"
namespace noi {

class ExchangeBuffer{
  public:
  std::mutex mutex;
  float dry_wet;
  float feedback;
  ExchangeBuffer() : dry_wet{0}, feedback{0}, mutex{} {}
};

class Philodendron {
 public:
  /// @brief Parameters of a stereoMoorer Reverb
  /// @param freeze
  /// @param drywet from 0 to 1
  /// @param comb_time
  /// @param variation
  /// @param feedback
  struct Parameters {
    bool freeze;
    float dry_wet, comb_time, variation, feedback, nb_head, head_ratio, read_offset;
  };
  float prev_offset;
  Philodendron(noi::Philodendron::Parameters parameters, int sample_rate, std::shared_ptr<ExchangeBuffer>& _exchange_buffer);
  void reset(noi::Philodendron::Parameters parameters, int sample_rate);
  void updateParameters(noi::Philodendron::Parameters parameters);
  void setSampleRate(float sample_rate);
  std::array<float, 2> processStereo(std::array<float, 2> inputs);
  void setFreeze();
  void resize();
  void updateExchangeBuffer();

 private:
  std::array<noi::RingBuffer, 2> m_ring_buffers;
  std::shared_ptr<ExchangeBuffer> exchange_buffer;

  int update_exchange_buffer;

  noi::Philodendron::Parameters m_parameters;
  noi::Philodendron::Parameters m_old_parameters;
  std::array<float, 2> m_outputs{0, 0};
}; /*Philodendron*/

}  // namespace noi