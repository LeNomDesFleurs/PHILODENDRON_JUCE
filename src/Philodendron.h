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
  class Content{
    public:
  float feedback;
  float dry_wet;
  float read_ref;
  float write;
  float head_position;
  float read_speed;
  float head_ratio;
  float head_number;
  float distance;
  float read_offset;
  bool freezed;
  };

  std::mutex mutex;
  Content content;
};

	// first order FirstOrderFilter



class FirstOrderFilter {
	private:
    // compute coefficient
		float alpha{0};
    //previous sample
		float yz{0};
		float m_freq{ 8000 };
    float sampleRate{41000.f};

    public:

    void setSampleRate(float _sampleRate){
      sampleRate = _sampleRate;
      updateCoef();
    }

    void updateCoef(){
			float omega = 2.f * M_PI * m_freq;
			alpha = 1.f / ((sampleRate / omega) + 1.f);
    }

		void setParam(float freq)
		{
			if (m_freq == freq)
			{
				return;
			}
			m_freq = freq;
      updateCoef();
    }

    float processlpf(float x)
		{
			float y = x * alpha + yz * (1 - alpha);
			yz = y;
			return y;
		}
float processhpf(float sample) { return sample + (-1 * processlpf(sample)); }
		FirstOrderFilter(float freq, float _sampleRate)
    :sampleRate {_sampleRate}
     { setParam(freq); }
    FirstOrderFilter();

	}; /*FirstOrderFilter*/


class Philodendron {
 public:
  /// @brief Parameters of a stereoMoorer Reverb
  /// @param freeze
  /// @param drywet from 0 to 1
  /// @param comb_time
  /// @param read_speed
  /// @param feedback
  struct Parameters {
    bool freeze;
    float dry_wet, comb_time, read_speed, feedback, nb_head, head_ratio, read_offset;
  };
  Philodendron(noi::Philodendron::Parameters parameters, int sample_rate, std::shared_ptr<ExchangeBuffer>& _exchange_buffer);
  void reset(noi::Philodendron::Parameters parameters, int sample_rate);
  void updateParameters(noi::Philodendron::Parameters parameters);
  void setSampleRate(float sample_rate);
  std::array<float, 2> processStereo(std::array<float, 2> inputs);
  void setFreeze();
  void resize();
  void updateExchangeBuffer();

 private:
  noi::StereoRingBuffer m_ring_buffer;
  std::shared_ptr<ExchangeBuffer> exchange_buffer;

  int update_exchange_buffer {};

  float prev_offset{};

  std::vector<noi::FirstOrderFilter> hpf;
  noi::Philodendron::Parameters m_parameters;
  noi::Philodendron::Parameters m_old_parameters;
  std::array<float, 2> m_outputs{0, 0};
}; /*Philodendron*/

}  // namespace noi