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

  variationAttachment.reset(new Attachment(vts, "read_speed", readSpeedSlider));
  feedbackAttachment.reset(new Attachment(vts, "feedback", feedbackSlider));
  combSizeAttachement.reset(new Attachment(vts, "comb_time", bufferSizeSlider));
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

  setSize(positions.diameter, positions.diameter);
  // load Image from BinaryData
  // svgimg = juce::Drawable::createFromImageData(BinaryData::noi_svg,
  //                                              BinaryData::noi_svgSize);
}

PhilodendronEditor::~PhilodendronEditor() {}

std::vector<juce::Slider*> PhilodendronEditor::getComps() {
  return {
      &dryWetSlider,
      &feedbackSlider,
      // &readSpeedSlider,
      // &bufferSizeSlider,
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
  readSpeedSlider.setBounds(positions.read_speed.toNearestInt());
  bufferSizeSlider.setBounds(positions.buffer_size.toNearestInt());
  headRatioSlider.setBounds(positions.head_ratio.toNearestInt());
  readOffsetSlider.setBounds(positions.read_offset.toNearestInt());
  nbHeadSlider.setBounds(positions.head_number.toNearestInt());

  feedbackSlider.setBounds(positions.feedback.toNearestInt());
  dryWetSlider.setBounds(positions.dry_wet.toNearestInt());
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}

void PhilodendronEditor::updateFromExchangeBuffer(){
    if(this->exchange_buffer->mutex.try_lock()){
      this->parameters = exchange_buffer->content;
      exchange_buffer->mutex.unlock();
  }
}

void PhilodendronEditor::timerCallback() {
  // repaint_ui = true;

  repaint();
}

void PhilodendronEditor::paintDryWetWidget(juce::Graphics& g) { 
  juce::Path p;

  p.addPieSegment(positions.dry_wet, 0.f, (2 * M_PI)*this->parameters.dry_wet, 0.f);


  juce::PathStrokeType stroke =
      PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);
  g.strokePath(p, stroke);
  }

  void PhilodendronEditor::paintReadSpeed(juce::Graphics& g) { 
  juce::Path p;

  p.addPieSegment(positions.feedback, 0.f, (2.f * M_PI)*this->parameters.feedback, 0.f);


  juce::PathStrokeType stroke = PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);
  g.strokePath(p, stroke);
  }

void PhilodendronEditor::paintFeedbackWidget(juce::Graphics& g){
  float centerx = positions.feedback.getCentreX();
  float centery = positions.feedback.getCentreY();
  float radius = positions.feedback.getWidth()/2.f;


  float bottom_offset = 0.08;
  float offset_time = this->parameters.feedback * (1.0 - bottom_offset) + bottom_offset;

  // juce::Colour colour = CustomColors::getGradientWithoutGreen(this->parameters.feedback);
  // colour = CustomColors::fadeToDefault(colour, dry_wet);
  g.setColour(juce::Colours::black);

  float radian_goal = cheappi * offset_time;

  float offset = 0.5;
  // float offset = parameters.read_ref;
  

  juce::Path p;
  p.addCentredArc(centerx, centery, radius, radius, offset, 0.0,
                  radian_goal, true);
  p.addCentredArc(centerx, centery, radius, radius,
                  offset + cheappi, 0.0, radian_goal, true);
  juce::PathStrokeType stroke =
      PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);

  // the arrow disapear when the circle become complete
  float distance_complete_circle = pow(1.0 - offset_time, 0.3);
  float arrow_width = distance_complete_circle * 6.0;
  float arrow_height = distance_complete_circle * 10.0;

  stroke.PathStrokeType::createStrokeWithArrowheads(p, p, 0.0, 0.0, arrow_width,
                                                    arrow_height);


  g.strokePath(p, stroke);

}
