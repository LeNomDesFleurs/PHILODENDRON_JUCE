#include "RingBuffer.hpp"

namespace noi {

/// @brief constructor for ring buffer
/// @param max_time in seconds
/// @param initial_delay in seconds
StereoRingBuffer::StereoRingBuffer(float max_time, float initial_delay, int _sample_rate)
    : sample_rate{_sample_rate},
      m_write{(int)(initial_delay * (float)sample_rate)},
      m_actual_size{(float)m_write},
      m_crossfade_buffer(CROSSFADE_SIZE, 0),
      m_buffers{*new std::vector<float>(max_time * (float)sample_rate, 0), *new std::vector<float>(max_time * (float)sample_rate, 0)},
      m_buffer_size{(int)m_buffers[0].size() - 1} {}

void StereoRingBuffer::reset(float max_time, float initial_delay, int _sample_rate) {
  sample_rate = _sample_rate;
  m_write = (int)(initial_delay * (float)sample_rate);
  m_actual_size = (float)m_write;
  for (int i = 0; i < 2; i++){ 
    m_buffers[i].resize(max_time * (float)sample_rate);
    fill(m_buffers[i].begin(), m_buffers[i].end(), 0.f);
  }
  m_buffer_size = (int)m_buffers.size() - 1;
}
void StereoRingBuffer::setStepSize(float step_size) { m_step_size = step_size; }

/// @brief Take a delay time in milliseconds, clip it within the defined max
/// buffer size and set the goal to reach.
/// @param delay_time in milliseconds
void StereoRingBuffer::setDelayTime(float delay_time) {
  new_size = true;
  float delay_in_samples =
      noi::Outils::convertMsToSample(delay_time * 1000.f, sample_rate);
  m_size_goal =
      noi::Outils::clipValue(delay_in_samples, 10, m_buffer_size - 1);
  if (m_buffer_mode == freeze){
    m_actual_size = noi::Outils::clipValue(delay_in_samples, 10, m_buffer_size - 1);
  }
}

void StereoRingBuffer::setReadOffset(float offset){
  m_read_offset = ((float)m_buffer_size * offset);
}

void StereoRingBuffer::setSampleRate(float _sample_rate) {
  sample_rate = _sample_rate;
}

void StereoRingBuffer::setHeadsReadSpeed(float base_read_speed, float ratio){
  heads[0].read_speed = base_read_speed;
  for (int i = 1; i < NUMBER_OF_HEADS; i++) {
    heads[i].read_speed = heads[i - 1].read_speed * ratio;
  }
}

void StereoRingBuffer::setFreezed(bool freezed) {
  // avoid updating the m_size_on_freeze
  // if (m_buffer_mode != freeze && m_buffer_mode != accumulate && freezed) {
  //   m_size_on_freeze = getActualSize();
  //   m_buffer_mode = accumulate;
  // }
  // if (!freezed) m_buffer_mode = normal;

  m_buffer_mode = freezed ? freeze : normal;
}

void StereoRingBuffer::freezedUpdateStepSize() {
  float step_size_goal = m_size_on_freeze / m_size_goal;
  m_step_size = noi::Outils::slewValue(step_size_goal, m_step_size, 0.9999);
}

void StereoRingBuffer::checkForReadIndexOverFlow() {
  while (m_read_reference < 0 || m_read_reference > m_buffer_size){
  if (m_read_reference < 0) {
    m_read_reference += m_buffer_size;
  }
  if (m_read_reference > m_buffer_size) {
    m_read_reference -= m_buffer_size;
  }
  }

    while (m_read < 0 || m_read > m_buffer_size){
  if (m_read < 0) {
    m_read += m_buffer_size;
  }
  if (m_read > m_buffer_size) {
    m_read -= m_buffer_size;
  }
  }
}

float StereoRingBuffer::getActualSize() {
  float temp_read = m_read_reference;
  if (m_read_reference > (float)m_write) temp_read -= m_buffer_size;
  float output = (float)m_write - temp_read;
  return output;
}

/// @brief increment read pointer and return sample from interpolation
std::array<float, 2> StereoRingBuffer::readSample() {
  if (m_buffer_mode == reverse) {
    m_step_size = 0. - m_step_size;
  }

  m_output_samples = {0.f, 0.f};

    if (m_buffer_mode == freeze) {
      freezeIncrementReadPointerReference();
      freezedUpdateStepSize();
    } else {
      updateStepSize();
      incrementReadPointerReference();
    }

    for (int channel = 0; channel<2; channel++){

      for (int i = 0; i < active_heads; i++) {
        heads[i].increment(m_actual_size);

        m_read = m_read_reference + heads[i].distance;

        if (m_buffer_mode == freeze) {
          m_read += m_read_offset;
        }

        float sample = interpolate(channel);
        // those functions modify the m_output_sample value

        // reverse crossfade
        if (heads[i].distance < CROSSFADE_SIZE && heads[i].read_speed < 0.f) {
          m_read = (float)m_write - heads[i].distance;
          float crossfade_sample = interpolate(channel);
          sample = noi::Outils::equalPowerCrossfade(
              sample, crossfade_sample,
              heads[i].distance / (float)CROSSFADE_SIZE);
        }
        // crossfade
        else if ((m_actual_size - heads[i].distance) < CROSSFADE_SIZE &&
                 heads[i].read_speed > 0.f) {
          m_read =
              m_read_reference + ((float)m_actual_size - heads[i].distance);
          float crossfade_sample = interpolate(channel);
          sample = noi::Outils::equalPowerCrossfade(
              sample, crossfade_sample,
              (m_actual_size - heads[i].distance) / (float)CROSSFADE_SIZE);
        }

        // if (m_buffer_mode == freeze) {
        //   m_output_sample *=
        //       noi::Outils::clipValue(1.f / pow(m_step_size, 0.5), 0.2, 3.);
        // }

        m_output_samples[channel] += sample;
      }
  m_output_samples[channel] /= (float)active_heads;
}

  return m_output_samples;
}

float StereoRingBuffer::interpolate(int index){
  checkForReadIndexOverFlow();
  fractionalizeReadIndex();
  switch (interpolation_mode) {
    case none:
      return noInterpolation(index);
      break;
    case linear:
      return linearInterpolation(index);
      break;
    case allpass:
      return allpassInterpolation(index);
      break;
  }
}

/// @brief Triggered at each sample, update the step size and the m_actual_size
/// to keep up with change of size goal
void StereoRingBuffer::updateStepSize() {
  float step_size_goal = 1.0;
  m_actual_size = getActualSize();
  // big sample limit to account for inertia
  if ((m_actual_size > m_size_goal) && new_size) {
    // m_step_size = 1.5;
    step_size_goal = 2.0;
    // update the step size but with slew for clean repitch
  } else if ((m_actual_size < m_size_goal) && new_size) {
    // m_step_size = 0.5;
    step_size_goal = 0.5;
  }

  m_step_size =
      noi::Outils::slewValue(step_size_goal, m_step_size,
                             0.999);  // should be modified by sample rate
  if (m_actual_size > (m_size_goal - 200) &&
      m_actual_size < (m_size_goal + 200)) {
    if (ready_to_lock) m_step_size = 1.0;
    new_size = false;
  } else {
    ready_to_lock = true;
  }
}
/// @brief increment pointer and set its int, incremented int and frac value
void StereoRingBuffer::incrementReadPointerReference() {
  m_read_reference += m_step_size;
  checkForReadIndexOverFlow();
}
void StereoRingBuffer::fractionalizeReadIndex() {
  // get sample
  m_i_read = static_cast<int>(trunc(m_read));
  // get fraction
  m_frac = m_read - static_cast<float>(m_i_read);
  // Get next sample
  m_i_read_next = (m_i_read + 1) > m_buffer_size ? 0 : (m_i_read + 1);
}

void StereoRingBuffer::freezeIncrementReadPointerReference() {
  // m_read_reference += m_step_size;
  // buffer over and under flow
  // checkForReadIndexOverFlow();
  // m_actual_size -= m_step_size;

  // In freezed case, m_read only iterate on the last buffer size,
  //  hence it's like a little ringBuffer in the bigger ringBuffer
  //  so more buffer over and under flow
  // if (m_actual_size < 0.) {
  //   m_read -= (float)m_write - m_size_on_freeze;
  //   checkForReadIndexOverFlow();
  //   m_actual_size = m_size_on_freeze;

  // } else if (m_actual_size > m_size_on_freeze) {
  //   m_read = (float)m_write;
  //   m_actual_size = 0;
  // }
}

float StereoRingBuffer::noInterpolation(int index) { return m_buffers[index][m_i_read]; }

/// @brief Interpolation lineaire du buffer a un index flottant donne
float StereoRingBuffer::linearInterpolation(int index) {
  // S[n]=frac * Buf[i+1]+(1-frac)*Buf[i]

  return (m_frac * m_buffers[index][m_i_read_next]) + ((1 - m_frac) * m_buffers[index][m_i_read]);


}

      // m_output_sample = m_buffer[(int)m_read_reference];


/// @brief Interpolation passe-tout, recursion
float StereoRingBuffer::allpassInterpolation(int index) {
  // S[n]=Buf[i+1]+(1-frac)*Buf[i]-(1-frac)*S[n-1]
  return (m_buffers[index][m_i_read + 1]) +
                    ((1 - m_frac) * m_buffers[index][m_i_read]) -
                    ((1 - m_frac) * m_output_samples[index]);
}

/// @brief increment write pointer and write input sample in buffer
/// @param input_sample
void StereoRingBuffer::writeSample(std::array<float, 2> input_samples) {
  // if (m_buffer_mode == normal || m_buffer_mode == reverse) {
    m_write = (m_write + 1) % m_buffer_size;
    for (int i = 0; i < 2; i++) 
    {
    m_buffers[i][m_write] = input_samples[i];
    }
    // m_buffer[0] = input_sample;
  // } else if (m_buffer_mode == accumulate) {
  //   if (accumulate_count != CROSSFADE_SIZE) {
  //     m_crossfade_buffer[accumulate_count] = input_sample;
  //     accumulate_count++;
  //   } else {
  //     m_buffer_mode = freeze;
  //     crossfade();
  //     accumulate_count = 0;
  //   }
  // }
}

void StereoRingBuffer::crossfade() {
  for (int buffer_channel = 0; buffer_channel < 2; buffer_channel++){
    for (int i = CROSSFADE_SIZE - 1; i >= 0; i--) {
      int buffer_index = (m_write + i + 1) % m_buffer_size;
      float coef = (float)i / (float)(CROSSFADE_SIZE - 1);

      // std::cout<< m_crossfade_buffer[i]<<'-' << m_buffer[buffer_index]<< '-'
      // <<coef;
      m_buffers[buffer_channel][buffer_index] = noi::Outils::linearCrossfade(
          m_crossfade_buffer[i], m_buffers[buffer_channel][buffer_index], coef);
      // std::cout << '\n';
    }}
}


}  // namespace noi