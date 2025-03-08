/*
  ==============================================================================

    Hellebore.cpp
    Created: 11 Mar 2023 2:28:30pm
    Author:  thoma

  ==============================================================================
*/

#include "Philodendron.h"
static const float MAX_BUFFER_SIZE = 8.f;

namespace noi {
using noi::StereoRingBuffer;

Philodendron::Philodendron(noi::Philodendron::Parameters parameters, int sample_rate, std::shared_ptr<noi::ExchangeBuffer>& _exchange_buffer)
: m_ring_buffer { StereoRingBuffer(MAX_BUFFER_SIZE, 2.f, sample_rate) }
, exchange_buffer {_exchange_buffer}
, hpf {FirstOrderFilter(20.f, 44100.f), FirstOrderFilter(50.f, 44100.f)}
, m_parameters {parameters}
, m_old_parameters {parameters}
{
  updateParameters(parameters);
  // m_allpasses[0].setGain(0.9);
  // m_allpasses[1].setGain(0.9);
}

void Philodendron::reset(noi::Philodendron::Parameters parameters, int sample_rate){
  m_ring_buffer.reset(8.f, 2.f, sample_rate);
  hpf[0].setSampleRate(sample_rate);
  hpf[1].setSampleRate(sample_rate);
  updateParameters(parameters);
}

void Philodendron::updateParameters(noi::Philodendron::Parameters parameters) {
  m_parameters = parameters;
  // setTime();     // time
  setFreeze();   // freeze
  resize();  // read_speed + comb

  prev_offset = noi::Outils::slewValue(m_parameters.read_offset, prev_offset, 0.95);

  for (int i = 0; i != 2; i++)
  {
    m_ring_buffer.setReadOffset(prev_offset);
    m_ring_buffer.active_heads = m_parameters.nb_head;
    m_ring_buffer.setHeadsReadSpeed(m_parameters.read_speed, m_parameters.head_ratio);
  }
    // setPan();      // read_speed
    m_old_parameters = m_parameters;
}

void Philodendron::setFreeze() {
  for (int i = 0; i < 2; i++) {
      m_ring_buffer.setFreezed(m_parameters.freeze);
  }
}

void Philodendron::updateExchangeBuffer(){
  if(this->exchange_buffer->mutex.try_lock()){
    exchange_buffer->content.dry_wet = this->m_parameters.dry_wet;
    exchange_buffer->content.feedback = this->m_parameters.feedback;
    exchange_buffer->content.read_speed = this->m_parameters.read_speed;
    exchange_buffer->content.read_ref =
        (float)this->m_ring_buffer.m_read_reference/
    (float)this->m_ring_buffer.m_buffer_size;
    exchange_buffer->content.write = 
                                     (float)this->m_ring_buffer.m_write/
    (float)this->m_ring_buffer.m_buffer_size;
    exchange_buffer->content.head_ratio = this->m_parameters.head_ratio;
    exchange_buffer->content.distance =
        (float)this->m_ring_buffer.heads[0].distance / (float)m_ring_buffer.m_actual_size;
    exchange_buffer->content.head_number = this->m_parameters.nb_head;
    exchange_buffer->content.freezed = this->m_parameters.freeze;
    exchange_buffer->content.read_offset = this->m_parameters.read_offset;
    exchange_buffer->content.size_goal =
        (float)this->m_ring_buffer.m_size_goal /
        (float)m_ring_buffer.m_buffer_size;
    exchange_buffer->mutex.unlock();
  }
}

void Philodendron::resize() {
  for (int i = 0; i < 2; i++) {
    float time = m_parameters.comb_time;
    m_ring_buffer.setDelayTime(time);
  }
}

void Philodendron::setSampleRate(float sample_rate) {
  for (int i = 0; i < 2; i++) {
    m_ring_buffer.setSampleRate(sample_rate);
  }
}


std::array<float, 2> Philodendron::processStereo(std::array<float, 2> inputs) {
  
  if (update_exchange_buffer == 1000){
    update_exchange_buffer = 0;
    updateExchangeBuffer();
  }
  update_exchange_buffer++;

    m_outputs = m_ring_buffer.readSample();

    m_outputs = noi::Outils::equalPowerCrossfade(inputs, m_outputs,
                                                  m_parameters.dry_wet);
    std::array<float, 2> feedback;
    for (int i = 0; i < 2; i++) {
      feedback[i] = inputs[i] + (m_outputs[i] * m_parameters.feedback);
      feedback[i] = hpf[i].processhpf(feedback[i]);
      if (feedback[i] <= -1.f) feedback[i] = -1.f;
      if (feedback[i] >= 1.f) feedback[i] = 1.f;
      if (m_outputs[i] <= -1.f) m_outputs[i] = -1.f;
      if (m_outputs[i] >= 1.f) m_outputs[i] = 1.f;

  
    }
    if (!m_parameters.freeze){
      m_ring_buffer.writeSample(feedback);
    }
  return m_outputs;
}

}  // namespace noi
