/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Philodendron.h"
using namespace juce;
//==============================================================================
/**
 */
using juce::Slider;

// class Encoder: public Slider::Pimpl, public juce::Component{
//  void handleAbsoluteDrag(const MouseEvent& e) override {
//     auto mousePos = (isHorizontal() || style == RotaryHorizontalDrag) ? e.position.x : e.position.y;
//         double newPos = 0;

//             auto mouseDiff = (style == RotaryHorizontalDrag
//                                 || style == LinearHorizontal
//                                 || style == LinearBar
//                                 || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
//                               ? e.position.x - mouseDragStartPos.x
//                               : mouseDragStartPos.y - e.position.y;

//             newPos = owner.valueToProportionOfLength (valueOnMouseDown)
//                        + mouseDiff * (1.0 / pixelsForFullDragExtent);

//             if (style == IncDecButtons)
//             {
//                 incButton->setState (mouseDiff < 0 ? Button::buttonNormal : Button::buttonDown);
//                 decButton->setState (mouseDiff > 0 ? Button::buttonNormal : Button::buttonDown);
//             }
        

//         newPos = (isRotary() && ! rotaryParams.stopAtEnd) ? newPos - std::floor (newPos)
//                                                           : jlimit (0.0, 1.0, newPos);
//         valueWhenLastDragged = owner.proportionOfLengthToValue (newPos);
//   }
// };
class Positions {
public:
int diameter;
float slider_thickness;
float center;
juce::Rectangle<float> read_offset;
juce::Rectangle<float> title;
juce::Rectangle<float> buffer_size;
juce::Rectangle<float> head_ratio;
juce::Rectangle<float> read_speed;
juce::Rectangle<float> head_number;
juce::Rectangle<float> feedback;
juce::Rectangle<float> dry_wet;
 
std::vector<juce::Rectangle<float>*> positions;

Positions(int diameter){
  
  // first component will be the outer component, growing inward
  positions.insert(positions.end(), {
    &title,
    &read_offset, 
    &buffer_size,
    &read_speed,
    &feedback,
    &head_ratio,
    &head_number,
    &dry_wet
  });
    setDiameter(diameter);
 }
    void setDiameter(int _diameter){
        diameter = _diameter;
        center = diameter/2.f;
        slider_thickness = (diameter / 2) / positions.size();
        float current_radius = diameter / 2;

        for (auto* position : positions){
          float x = (diameter / 2) - current_radius;
          float width = current_radius * 2;
          *position = juce::Rectangle<float>(x, x, width, width);
          current_radius -= slider_thickness;
        }
    }
    
};

class circleSlider : public juce::Slider {
  bool hitTest(int x, int y) override {
    int h = this->getWidth() / 2;
    int k = this->getHeight() / 2;
    // disk equation
    // (x-h)^2 + (y-k)^2 < r^2
    // (h, k) the center of the circle, r it radius
    // (x, y) the position to test
    bool is_hit = (x - h) * (x - h) + (y - k) * (y - k) <= (h * h);
    return is_hit;
  }
};

class BackgroundComponent : public juce::Component {
  void paint(juce::Graphics& g){
  g.fillAll(juce::Colours::white);
  g.setColour(juce::Colours::black);
    
      g.setFont(juce::FontOptions("Times New Roman", 50.0f, juce::Font::italic));

  // Draw Logo
  const auto svg = Drawable::createFromImageData(BinaryData::NOI_svg,
                                                 BinaryData::NOI_svgSize);

  float logo_size = 30.f;
  juce::Rectangle<float> position = {positions->center - logo_size/2.f, (positions->center *2) -logo_size - 10.f, logo_size, logo_size};
  juce::RectanglePlacement placement = (36);
  svg->setTransformToFit(position, placement);
  svg->draw(g, 1.0);
  std::vector<std::string> title = {"P","h" ,"i", "l", "o", "d", "e", "n", "d", "r", "o", "n"};

    float angle = 1.f/((float)title.size() + 2);
    angle *= (M_PI * 2);

    float global_offset = (((float)title.size() / -2.f)-0.5)*angle;
    g.addTransform(juce::AffineTransform().rotated(global_offset, positions->center, positions->center));

  for (int i = 0; i < title.size(); i++){

    g.addTransform(juce::AffineTransform().rotated(angle, positions->center, positions->center));
  g.drawFittedText(title[i], positions->center - positions->slider_thickness /2.f, 0, positions->slider_thickness, positions->slider_thickness,
                   juce::Justification::centred, 1);
  }

  //DRYWET background
  juce::Path circle;
  circle.addEllipse(
      positions->dry_wet.reduced(positions->slider_thickness / 2.f).expanded(10, 10));
  g.strokePath(circle, thin_stroke);


  //READ_SPEED BACKGROUND
  juce::Path read_speed_circle;
  read_speed_circle.addEllipse(
      positions->read_speed.reduced(positions->slider_thickness / 2.f));
  float dashes[4] = {1.f, 20.f};
  thin_stroke.createDashedStroke(read_speed_circle, read_speed_circle, dashes, 2);
  g.strokePath(read_speed_circle, thin_stroke);

  //BUFFER_SIZE BACKGROUND
  juce::Path buffer_size_circle;
  buffer_size_circle.addEllipse(
      positions->buffer_size.reduced(positions->slider_thickness / 2.f));
  float dashes_buffer_size[2] = {10.f, 10.f};
  thin_stroke.createDashedStroke(buffer_size_circle, buffer_size_circle, dashes_buffer_size, 1);
  g.strokePath(buffer_size_circle, thin_stroke);

juce::Path feedback_circle;
  feedback_circle.addEllipse(
      positions->feedback.reduced(positions->slider_thickness / 2.f));
  float dashes_feedback[4] = {7.f, 10.f, 1.f, 10.f};
  thin_stroke.createDashedStroke(feedback_circle, feedback_circle, dashes_feedback, 4);
  g.strokePath(feedback_circle, thin_stroke);

  
  }
  public:
  Positions* positions;
  juce::PathStrokeType thin_stroke {PathStrokeType(0.5, PathStrokeType::curved, PathStrokeType::rounded)};
};

class EmptyKnob : public juce::LookAndFeel_V4 {
 public:
  EmptyKnob() {};
  void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider& slider) override {};
  void drawLinearSlider(Graphics&, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        Slider::SliderStyle, Slider&) override {};
};

class PlaceHolder : public juce::LookAndFeel_V4 {
 public:
  PlaceHolder() {};
  void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle,
                        juce::Slider& slider) override {
    juce::Path p;
    p.addRectangle(x, y, width, height);
    g.setColour(juce::Colour::fromString("40800000"));
    g.fillPath(p);
  };
};


class PhilodendronEditor : 
public juce::AudioProcessorEditor,
// public juce::AudioProcessorParameter::Listener,
public juce::Timer{
 public:
  PhilodendronEditor(PhilodendronProcessor &p, juce::AudioProcessorValueTreeState& vts, std::shared_ptr<noi::ExchangeBuffer>& _exchange_buffer);
  ~PhilodendronEditor() override;

  // void parameterValueChanged(int parameterIndex, float newValue) override;
  // void parameterGestureChanged(int parameterIndex,
  //                              bool gestureIsStarting) override {}
  //==============================================================================
  // void paint(juce::Graphics &) override;
  void updateFromExchangeBuffer();
  void timerCallback() override;
  void resized() override;
  void paintOverChildren(juce::Graphics& g) override;
  std::vector<juce::Slider *> getComps();

  EmptyKnob emptyKnobLookAndFeel;
  PlaceHolder placeHolderLookAndFeel;

  Positions positions;

  void paintDryWetWidget(juce::Graphics &g);
  void paintFeedbackWidget(juce::Graphics &g);
  void paintReadSpeed(juce::Graphics& g);
  void paintReadOffset(juce::Graphics& g);

  void paintHeadRatio(juce::Graphics& g);
  void paintNumberOfHeads(juce::Graphics& g);
  void paintBufferSize(juce::Graphics& g);


  // juce::Line<float> 
  void buildRadiusSegment(float center_x, float center_y,
                                       float angle, float distance,
                                       float length, juce::Path& p);

ComponentBoundsConstrainer newConstrainer;
    
  circleSlider readSpeedSlider;
  circleSlider feedbackSlider;
  circleSlider bufferSizeSlider;
  circleSlider dryWetSlider;
  circleSlider nbHeadSlider;
  circleSlider headRatioSlider;
  circleSlider readOffsetSlider;
  BackgroundComponent backgroundComponent;

  using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
  std::unique_ptr<Attachment> variationAttachment;
  std::unique_ptr<Attachment> feedbackAttachment;
  std::unique_ptr<Attachment> combSizeAttachement;
  std::unique_ptr<Attachment> dryWetAttachement;
  std::unique_ptr<Attachment> nbHeadAttachement;
  std::unique_ptr<Attachment> headRatioAttachement;
  std::unique_ptr<Attachment> readOffsetAttachement;

  std::shared_ptr<noi::ExchangeBuffer> exchange_buffer;
  noi::ExchangeBuffer::Content parameters;
  juce::PathStrokeType stroke {PathStrokeType(2.0, PathStrokeType::curved, PathStrokeType::rounded)};
  juce::PathStrokeType arrow_stroke {PathStrokeType(0.001, PathStrokeType::curved, PathStrokeType::rounded)};
  juce::PathStrokeType thin_stroke {PathStrokeType(0.5, PathStrokeType::curved, PathStrokeType::rounded)};
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PhilodendronProcessor &audioProcessor;
  std::unique_ptr<juce::Drawable> svgimg;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhilodendronEditor)
};
