/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"

#include <cmath>

//==============================================================================
PhilodendronEditor::PhilodendronEditor(
    PhilodendronProcessor &p,
    juce::AudioProcessorValueTreeState& vts, 
    std::shared_ptr<noi::ExchangeBuffer>& _exchange_buffer)
    : AudioProcessorEditor(&p)
    , audioProcessor(p)
    , exchange_buffer {_exchange_buffer}
      {
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
  comp->setLookAndFeel(&emptyKnobLookAndFeel);
  comp->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  comp->setTextBoxStyle(juce::Slider::NoTextBox, false, 0,
                            comp->getTextBoxHeight());
  }


  const auto& params = audioProcessor.getParameters();
  for (auto param : params) {
    param->addListener(this);
  }

  startTimerHz(30);

  setSize(400, 500);
  // load Image from BinaryData
  // svgimg = juce::Drawable::createFromImageData(BinaryData::noi_svg,
  //                                              BinaryData::noi_svgSize);
}

PhilodendronEditor::~PhilodendronEditor() {}

std::vector<juce::Slider*> PhilodendronEditor::getComps() {
  return {
      &dryWetSlider,
      &feedbackSlider,
      // &variationSlider,
      // &combSizeSlider,
      // &nbHeadSlider,
      // &headRatioSlider,
      // &readOffsetSlider
  };
}

void PhilodendronEditor::parameterValueChanged(int parameterIndex, float newValue){}

//==============================================================================
void PhilodendronEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(juce::Colours::white);

  // svgimg->drawWithin(g, getLocalBounds().toFloat(),
  //                    juce::Justification::centred, 1);
  updateFromExchangeBuffer();
  paintDryWetWidget(g);
  paintFeedbackWidget(g);
  g.setColour(juce::Colours::black);
  g.setFont(30.0f);

}

void PhilodendronEditor::resized() {
  variationSlider.setBounds(10, 70, 300, 30);
  combSizeSlider.setBounds(10, 200, 300, 30);
  headRatioSlider.setBounds(10, 250, 300, 30);
  readOffsetSlider.setBounds(10, 300, 300, 30);
  nbHeadSlider.setBounds(10, 350, 300, 30);

  feedbackSlider.setBounds(30, 30, 110, 110);
  dryWetSlider.setBounds(10, 10, 140, 140);
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}

void PhilodendronEditor::updateFromExchangeBuffer(){
    if(this->exchange_buffer->mutex.try_lock()){
    this->dry_wet = exchange_buffer->dry_wet;
    this->feedback = exchange_buffer->feedback;
    exchange_buffer->mutex.unlock();
  }
}

void PhilodendronEditor::timerCallback() {
  // repaint_ui = true;

  repaint();
}

void PhilodendronEditor::paintDryWetWidget(juce::Graphics& g) { 
  juce::Path p;

  p.addPieSegment(10, 10, 140, 140, 0, (2 * M_PI)*this->dry_wet, 0.f);


  juce::PathStrokeType stroke =
      PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);
  g.strokePath(p, stroke);
  }

  void PhilodendronEditor::paintFeedbackWidget(juce::Graphics& g) { 
  juce::Path p;

  p.addPieSegment(30, 30, 100, 100, 0, (2 * M_PI)*this->feedback, 0.f);


  juce::PathStrokeType stroke =
      PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);
  g.strokePath(p, stroke);
  }