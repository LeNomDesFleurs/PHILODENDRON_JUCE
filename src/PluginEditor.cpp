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
  comp->setLookAndFeel(&placeHolderLookAndFeel);
  comp->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
  comp->setTextBoxStyle(juce::Slider::NoTextBox, false, 0,
                            comp->getTextBoxHeight());
  }

  // headRatioSlider.setLookAndFeel(&placeHolderLookAndFeel);
  // readOffsetSlider.setLookAndFeel(&placeHolderLookAndFeel);

  const auto& params = audioProcessor.getParameters();
  for (auto param : params) {
    param->addListener(this);
  }

  startTimerHz(60);

  setSize(positions.diameter, positions.diameter);
  // load Image from BinaryData
  // svgimg = juce::Drawable::createFromImageData(BinaryData::noi_svg,
  //                                              BinaryData::noi_svgSize);
}

PhilodendronEditor::~PhilodendronEditor() {}

std::vector<juce::Slider*> PhilodendronEditor::getComps() {
  return {
      &bufferSizeSlider,
      &readOffsetSlider,
      &headRatioSlider,
      &readSpeedSlider,
      &nbHeadSlider,
      &feedbackSlider,
      &dryWetSlider
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
  paintReadSpeed(g);
  paintReadOffset(g);
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

  float radius =
      this->positions.dry_wet.getHeight()/2.f - 4;
  float center = positions.center;

  p.addCentredArc( center, center, radius, radius, 0.f,  
  M_PI - (M_PI * this->parameters.dry_wet), M_PI + (M_PI * this->parameters.dry_wet), true);


  juce::PathStrokeType stroke =
      PathStrokeType(4.0, PathStrokeType::curved, PathStrokeType::rounded);
  g.strokePath(p, stroke);
  }

void PhilodendronEditor::paintReadSpeed(juce::Graphics& g) { 
  float centerx = this->positions.center;
  float centery = this->positions.center;
  float radius = positions.read_speed.getWidth()/2.f - (positions.slider_thickness / 2.f);


  float bottom_offset = 0.08;
  float read_speed = this->parameters.read_speed;
  read_speed = read_speed > 0 ? read_speed / 2.f : read_speed / 4.f;

  float offset_time = read_speed;  //* (1.0 - bottom_offset) + bottom_offset;

  // juce::Colour colour = CustomColors::getGradientWithoutGreen(this->parameters.feedback);
  // colour = CustomColors::fadeToDefault(colour, dry_wet);
  g.setColour(juce::Colours::black);

  float radian_goal = cheappi * offset_time * 2;

  // float offset = 0.5;
  float offset = parameters.read_ref;
  
  juce::Path p;
  p.addCentredArc(centerx, centery, radius, radius, offset, 0.0,
                  radian_goal, true);
  
  juce::PathStrokeType stroke =
      PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);

  // the arrow disapear when the circle become complete
  float distance_complete_circle = pow(1.0 - offset_time, 0.3);
  float arrow_width = 10.f;  // distance_complete_circle * 6.0;
  float arrow_height = 10.f; //distance_complete_circle * 10.0;

  stroke.PathStrokeType::createStrokeWithArrowheads(p, p, 0.0, 0.0, arrow_width,
                                                    arrow_height);


  g.strokePath(p, stroke);
  }

  void PhilodendronEditor::paintReadOffset(juce::Graphics& g) { 
    
    juce::Path p;

    float hypotenuse = (positions.read_offset.getHeight()/2.f);
    float theta1 = 2 * M_PI * parameters.read_ref;
    float theta2 = 2 * M_PI * parameters.write;

    juce::Line<float> read_head = buildRadiusSegment(positions.center, positions.center, -theta1, hypotenuse, positions.slider_thickness);
    juce::Line<float> write_head = buildRadiusSegment(positions.center, positions.center, -theta2, hypotenuse, positions.slider_thickness);

    p.addLineSegment(read_head, 2);
    p.addLineSegment(write_head, 2);

float centerx = this->positions.center;
    float centery = this->positions.center;
    float radius = positions.buffer_size.getWidth() / 2.f -
                   (positions.slider_thickness / 2.f);

 if (theta2 < theta1){
      theta2 += 2 * M_PI;
    }

p.addCentredArc(centerx, centery, radius, radius, M_PI , theta1, theta2,
                    true);


juce::PathStrokeType stroke =
    PathStrokeType(4.0, PathStrokeType::curved, PathStrokeType::rounded);

    juce::PathStrokeType stroke2 =
    PathStrokeType(0.5, PathStrokeType::curved, PathStrokeType::rounded);
juce::Path p2;
p2.addEllipse(positions.buffer_size.reduced(positions.slider_thickness/2.f));
float dashes[2] = {10.f, 10.f};
stroke2.createDashedStroke(p2, p2, dashes, 1);
g.strokePath(p, stroke);
g.strokePath(p2, stroke2);

juce::Path p3;
p3.addEllipse(positions.read_offset.reduced(positions.slider_thickness / 2.f));
g.strokePath(p3, stroke2);
    }

  void PhilodendronEditor::paintBufferSize(juce::Graphics& g) {


  }

  void PhilodendronEditor::paintFeedbackWidget(juce::Graphics& g) {
    float centerx = this->positions.center;
    float centery = this->positions.center;
    float radius = positions.feedback.getWidth() / 2.f - (positions.slider_thickness /2.f);
    // -(positions.slider_thickness / 2.f);

    float bottom_offset = 0.08;
    float offset_time =
        this->parameters.feedback * (1.0 - bottom_offset) + bottom_offset;

    // juce::Colour colour =
    // CustomColors::getGradientWithoutGreen(this->parameters.feedback); colour
    // = CustomColors::fadeToDefault(colour, dry_wet);
    g.setColour(juce::Colours::black);

    float radian_goal = cheappi * offset_time;

    // float offset = 0.5;
    float offset = parameters.read_ref * 2 * M_PI;

    juce::Path p;
    p.addCentredArc(centerx, centery, radius, radius, offset, 0.0, radian_goal,
                    true);
    p.addCentredArc(centerx, centery, radius, radius, offset + cheappi, 0.0,
                    radian_goal, true);
    juce::PathStrokeType stroke =
        PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded);

    // the arrow disapear when the circle become complete
    float distance_complete_circle = pow(1.0 - offset_time, 0.3);
    float arrow_width = distance_complete_circle * 6.0;
    float arrow_height = distance_complete_circle * 10.0;

    stroke.PathStrokeType::createStrokeWithArrowheads(
        p, p, 0.0, 0.0, arrow_width, arrow_height);

    g.strokePath(p, stroke);

}


juce::Line<float> PhilodendronEditor::buildRadiusSegment(float center_x, float center_y, float angle, float distance, float length){
  
    float start_x;
    float start_y;
    float end_x;
    float end_y;

    float hypotenuse = distance;
    float theta1 = angle;
    //read head
    start_x = center_x + sin(theta1) * hypotenuse;
    start_y = center_y + cos(theta1) * hypotenuse;

    hypotenuse -= length;

    end_x = center_x + sin(theta1) * hypotenuse;
    end_y = center_y + cos(theta1) * hypotenuse;

    return Line<float>(start_x, start_y, end_x, end_y);
}