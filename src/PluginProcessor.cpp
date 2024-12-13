/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PluginEditor.h"

//==============================================================================
PhilodendronProcessor::PhilodendronProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
,exchange_buffer{new noi::ExchangeBuffer()}
,philodendron{philodendron_parameters, 44100, exchange_buffer}
{
}

PhilodendronProcessor::~PhilodendronProcessor() {}

//==============================================================================
const juce::String PhilodendronProcessor::getName() const {
  return JucePlugin_Name;
}

bool PhilodendronProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool PhilodendronProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool PhilodendronProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double PhilodendronProcessor::getTailLengthSeconds() const { return 0.0; }

int PhilodendronProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int PhilodendronProcessor::getCurrentProgram() { return 0; }

void PhilodendronProcessor::setCurrentProgram(int index) {}

const juce::String PhilodendronProcessor::getProgramName(int index) {
  return {};
}

void PhilodendronProcessor::changeProgramName(int index,
                                                 const juce::String &newName) {}

//==============================================================================
void PhilodendronProcessor::prepareToPlay(double sampleRate,
                                             int samplesPerBlock) {
  philodendron.reset(philodendron_parameters, sampleRate);
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
}

void PhilodendronProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhilodendronProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void PhilodendronProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                            juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  philodendron.updateParameters(getSettings());

AudioPlayHead* phead = getPlayHead();

if (auto playposinfo = phead->getPosition()){
    // use *result

}

  if (totalNumInputChannels == 2) {
    // to access the sample in the channel as an array
    auto LeftChannelSamples = buffer.getWritePointer(0);
    auto RightChannelSamples = buffer.getWritePointer(1);

    for (auto n = 0; n < buffer.getNumSamples(); ++n) {
      std::array<float, 2> stereo_samples = {LeftChannelSamples[n],
                                             RightChannelSamples[n]};
      stereo_samples = philodendron.processStereo(stereo_samples);

      LeftChannelSamples[n] = stereo_samples[0];
      RightChannelSamples[n] = stereo_samples[1];
    }
  } else {
    auto LeftChannelSamples = buffer.getWritePointer(0);
    for (auto n = 0; n < buffer.getNumSamples(); ++n) {
      std::array<float, 2> stereo_samples = {LeftChannelSamples[n], 0.f};
      stereo_samples = philodendron.processStereo(stereo_samples);

      LeftChannelSamples[n] = stereo_samples[0];
    }
  }
}


noi::Philodendron::Parameters PhilodendronProcessor::getSettings() {
  noi::Philodendron::Parameters settings;

  settings.dry_wet = apvts.getRawParameterValue("dry_wet")->load();
  settings.comb_time = apvts.getRawParameterValue("buffer_size")->load();
  settings.read_speed = apvts.getRawParameterValue("read_speed")->load();
  settings.feedback = apvts.getRawParameterValue("feedback")->load();
  settings.head_ratio = apvts.getRawParameterValue("head_ratio")->load();
  settings.nb_head = apvts.getRawParameterValue("nb_head")->load();
  settings.read_offset = apvts.getRawParameterValue("read_offset")->load();

  
  // minimum feedback grows when comb feedback grows to keep some minimal
  // feedback max comb_time = 1.5 -> 1.5 * 5 settings.feedback =
  // noi::Outils::mapValue(feedback, 0.0, 1.0, settings.comb_time / 20.f, 20.);
  settings.freeze = settings.feedback >= 1.0;

  return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout PhilodendronProcessor::createParameterLayout(){
juce::AudioProcessorValueTreeState::ParameterLayout layout;

  using FloatRange = juce::NormalisableRange<float>;
  using FloatParam = juce::AudioParameterFloat;

  layout.add(std::make_unique<FloatParam>(
      "dry_wet", "Dry-Wet", FloatRange(0.f, 1.f, 0.01f, 1.f), 1.f));
  layout.add(std::make_unique<FloatParam>(
      "read_speed", "Read Speed", FloatRange(-4.f, 4.f, 0.000001f, 0.25f, true), 0.0f));
  layout.add(std::make_unique<FloatParam>(
      "buffer_size", "Buffer Size", FloatRange(0.0001f, 3.9f, 0.0001f, 0.3f), 1.f));
  layout.add(std::make_unique<FloatParam>(
      "feedback", "Feedback", FloatRange(0.0f, 1.0f, 0.00001f, 1.5f), 0.3f));
  layout.add(std::make_unique<FloatParam>(
    "read_offset", "Heads Offset", FloatRange(-1.f, 1.f, 0.00001f, 1.f), 0.f));
  layout.add(std::make_unique<FloatParam>(
      "head_ratio", "Head Ratio", FloatRange(-4.f, 4.f, 0.0001, 0.5, true), 1.f));
  layout.add(std::make_unique<FloatParam>(
    "nb_head", "Head Number", FloatRange(1.f, 4.f, 1.f, 1.f), 1.f));
  return layout;
}

//==============================================================================
bool PhilodendronProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PhilodendronProcessor::createEditor() {
  return new PhilodendronEditor(*this, apvts, exchange_buffer);
}

//==============================================================================
void PhilodendronProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void PhilodendronProcessor::setStateInformation(const void *data,
                                                   int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new PhilodendronProcessor();
}
