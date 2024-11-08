/*
  ==============================================================================

    Hellebore.cpp
    Created: 11 Mar 2023 2:28:30pm
    Author:  thoma

  ==============================================================================
*/

#include "Philodendron.h"
static const float MAX_COMB_SIZE = 4.f;

namespace noi {
using noi::RingBuffer;

Philodendron::Philodendron(noi::Philodendron::Parameters parameters, int sample_rate)
  : m_ring_buffers { {RingBuffer(4.f, 2.f, sample_rate), RingBuffer(4.f, 2.f, sample_rate)} }
  , m_old_parameters {parameters}
  , m_parameters {parameters}
{
  updateParameters(parameters);
  // m_allpasses[0].setGain(0.9);
  // m_allpasses[1].setGain(0.9);
}

void Philodendron::reset(noi::Philodendron::Parameters parameters, int sample_rate){
for (auto buffer : m_ring_buffers){
  buffer.reset(4.f, 2.f, sample_rate);
}
updateParameters(parameters);
}

void Philodendron::updateParameters(noi::Philodendron::Parameters parameters) {
  m_parameters = parameters;
  // setTime();     // time
  setFreeze();   // freeze
  resize();  // variation + comb
  

  for (int i = 0; i != 2; i++)
  {
    m_ring_buffers[i].setReadOffset(m_parameters.read_offset);
    m_ring_buffers[i].active_heads = m_parameters.nb_head;
    m_ring_buffers[i].setHeadsReadSpeed(m_parameters.variation, m_parameters.head_ratio);
  }
    // setPan();      // variation
    m_old_parameters = m_parameters;
}

// bool Philodendron::variationHaventChange() {
//   return m_parameters.variation == m_old_parameters.variation;
// }

// bool Philodendron::timeHaventChange() {
//   return m_parameters.feedback == m_old_parameters.feedback;
// }

// bool Philodendron::combSizeHaventChange() {
//   return m_parameters.comb_time == m_old_parameters.comb_time;
// }

// bool Philodendron::freezeHaventChange() {
//   return m_parameters.freeze == m_old_parameters.freeze;
// }


// more variation -> more diffrence of gain between combs
// void Philodendron::setTime() {
//   if (timeHaventChange()) return;
//   float feedback = m_parameters.feedback;
//   for (int i = 0; i < 2; i++) {
//     // m_allpasses[i].setGain(feedback);
//     for (int j = 0; j < 6; j++) {
//       m_combs[i][j].overrideFeedback(feedback);
//       ;
//     }
//   }
// }

void Philodendron::setFreeze() {
  for (int i = 0; i < 2; i++) {
      m_ring_buffers[i].setFreezed(m_parameters.freeze);
  }
}

// more variation -> more diffrence of gain between combs
void Philodendron::resize() {
  for (int i = 0; i < 2; i++) {
    float time = m_parameters.comb_time;
    m_ring_buffers[i].setDelayTime(time);
  }
}

void Philodendron::setSampleRate(float sample_rate) {
  for (int i = 0; i < 2; i++) {
    // process combs
    m_ring_buffers[i].setSampleRate(sample_rate);
  }
}


std::array<float, 2> Philodendron::processStereo(std::array<float, 2> inputs) {
  
  // Sum all combs for each channels
  for (int i = 0; i < 2; i++) {
    float output = m_ring_buffers[i].readSample();

    m_outputs[i] = 
    // ((2 * m_parameters.dry_wet) + 1) *
                 noi::Outils::equalPowerCrossfade(inputs[i], output,
                                                  m_parameters.dry_wet);
    if (m_outputs[i] > 1.f || m_outputs[i] < -1.f){
      m_outputs[i] = 0;
    }
    if (!m_parameters.freeze){
    m_ring_buffers[i].writeSample(inputs[i] + m_outputs[i] * m_parameters.feedback);
    }
  }
  return m_outputs;
}

}  // namespace noi
