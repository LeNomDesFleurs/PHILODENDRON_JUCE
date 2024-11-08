/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
PhilodendronEditor::PhilodendronEditor(
    PhilodendronProcessor &p,
    juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.


  using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

  variationAttachment.reset(new Attachment(vts, "variation", variationSlider));
  feedbackAttachment.reset(new Attachment(vts, "feedback", feedbackSlider));
  combSizeAttachement.reset(new Attachment(vts, "comb_time", combSizeSlider));
  dryWetAttachement.reset(new Attachment(vts, "dry_wet", dryWetSlider));
  nbHeadAttachement.reset(new Attachment(vts, "nb_head", nbHeadSlider));
  readOffsetAttachement.reset(new Attachment(vts, "read_offset", readOffsetSlider));
  headRatioAttachement.reset(new Attachment(vts, "head_ratio", headRatioSlider));

  // Background BEFORE widgets

  for (auto* comp : getComps()) {
    addAndMakeVisible(comp);
  }

  const auto& params = audioProcessor.getParameters();
  for (auto param : params) {
    param->addListener(this);
  }

  setSize(400, 500);
  // load Image from BinaryData
  // svgimg = juce::Drawable::createFromImageData(BinaryData::noi_svg,
  //                                              BinaryData::noi_svgSize);
}

PhilodendronEditor::~PhilodendronEditor() {}

std::vector<juce::Slider*> PhilodendronEditor::getComps() {
  return {
      &feedbackSlider,
      &variationSlider,
      &dryWetSlider,
      &combSizeSlider,
      &nbHeadSlider,
      &headRatioSlider,
      &readOffsetSlider
  };
}

void PhilodendronEditor::parameterValueChanged(int parameterIndex, float newValue){}

//==============================================================================
void PhilodendronEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(juce::Colours::black);
  // svgimg->drawWithin(g, getLocalBounds().toFloat(),
  //                    juce::Justification::centred, 1);

  g.setColour(juce::Colours::black);
  g.setFont(30.0f);

}

void PhilodendronEditor::resized() {
  feedbackSlider.setBounds(10, 10, 300, 30);
  variationSlider.setBounds(10, 70, 300, 30);
  dryWetSlider.setBounds(10, 140, 300, 30);
  combSizeSlider.setBounds(10, 200, 300, 30);
  headRatioSlider.setBounds(10, 250, 300, 30);
  readOffsetSlider.setBounds(10, 300, 300, 30);
  nbHeadSlider.setBounds(10, 350, 300, 30);

  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}
