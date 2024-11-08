/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"


//==============================================================================
/**
 */
class PhilodendronEditor : 
public juce::AudioProcessorEditor,
public juce::AudioProcessorParameter::Listener{
 public:
  PhilodendronEditor(PhilodendronProcessor &p, juce::AudioProcessorValueTreeState& vts);
  ~PhilodendronEditor() override;

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override {}
  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;
  std::vector<juce::Slider *> getComps();



  juce::Slider variationSlider;
  juce::Slider feedbackSlider;
  juce::Slider combSizeSlider;
  juce::Slider dryWetSlider;
  juce::Slider nbHeadSlider;
  juce::Slider headRatioSlider;
  juce::Slider readOffsetSlider;

  using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  std::unique_ptr<Attachment> variationAttachment;
  std::unique_ptr<Attachment> feedbackAttachment;
  std::unique_ptr<Attachment> combSizeAttachement;
  std::unique_ptr<Attachment> dryWetAttachement;
  std::unique_ptr<Attachment> nbHeadAttachement;
  std::unique_ptr<Attachment> headRatioAttachement;
  std::unique_ptr<Attachment> readOffsetAttachement;

  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PhilodendronProcessor &audioProcessor;
  std::unique_ptr<juce::Drawable> svgimg;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhilodendronEditor)
};
