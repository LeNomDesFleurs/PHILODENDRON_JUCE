/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Philodendron.h"
#include <array>

//==============================================================================
/**
 */
class PhilodendronProcessor : public juce::AudioProcessor {
 public:
  //==============================================================================
  PhilodendronProcessor();
  ~PhilodendronProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;
  noi::Philodendron::Parameters getSettings();
  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

   juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters",
                                           createParameterLayout()};
 private:
  noi::Philodendron::Parameters philodendron_parameters{false, 0.5F, 0.01f, 0.1f,
                                                     0.1f};
  noi::Philodendron philodendron;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhilodendronProcessor)
};
