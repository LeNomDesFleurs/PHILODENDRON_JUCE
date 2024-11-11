/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "Philodendron.h"

//==============================================================================
/**
 */

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

Positions() : diameter{400} , center{diameter/2.f}{ 
  
  // first component will be the outer component, growing inward
  positions.insert(positions.end(), {
    &title,
    &buffer_size,
    &read_offset, 
    &head_ratio,
    &read_speed,
    &head_number,
    &feedback,
    &dry_wet
  });

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
public juce::AudioProcessorParameter::Listener,
public juce::Timer{
 public:
  PhilodendronEditor(PhilodendronProcessor &p, juce::AudioProcessorValueTreeState& vts, std::shared_ptr<noi::ExchangeBuffer>& _exchange_buffer);
  ~PhilodendronEditor() override;

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex,
                               bool gestureIsStarting) override {}
  //==============================================================================
  void paint(juce::Graphics &) override;
  void updateFromExchangeBuffer();
  void timerCallback() override;
  void resized() override;
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

  juce::Line<float> buildRadiusSegment(float center_x, float center_y,
                                       float angle, float distance,
                                       float length);

  circleSlider readSpeedSlider;
  circleSlider feedbackSlider;
  circleSlider bufferSizeSlider;
  circleSlider dryWetSlider;
  circleSlider nbHeadSlider;
  circleSlider headRatioSlider;
  circleSlider readOffsetSlider;

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
  juce::PathStrokeType arrow_stroke {PathStrokeType(0.25, PathStrokeType::curved, PathStrokeType::rounded)};
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  PhilodendronProcessor &audioProcessor;
  std::unique_ptr<juce::Drawable> svgimg;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhilodendronEditor)
};
