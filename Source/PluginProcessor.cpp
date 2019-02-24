/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RepeaterPluginAudioProcessor::RepeaterPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

RepeaterPluginAudioProcessor::~RepeaterPluginAudioProcessor()
{
}

//==============================================================================
const String RepeaterPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RepeaterPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RepeaterPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RepeaterPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RepeaterPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RepeaterPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RepeaterPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RepeaterPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String RepeaterPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void RepeaterPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void RepeaterPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    notes.clear();
    currentlyPlayingNotes.clear();
    notesPerBar = 16;
    skipPercentage = 25;
    skipNext = false;
    multiplePercentages.insert(std::make_pair(2, 10));
    multiplePercentages.insert(std::make_pair(3, 6));
    multipleNext = 0;
}

void RepeaterPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RepeaterPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RepeaterPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }

    MidiMessage msg;
    int ignore;

    for (MidiBuffer::Iterator it(midiMessages); it.getNextEvent(msg, ignore); ) {
      if      (msg.isNoteOn())  {
        notes.add(msg.getNoteNumber());
        /* std::cout << "adding note " << msg.getNoteNumber() << "\n"; */
      }
      else if (msg.isNoteOff()) {
        notes.removeValue(msg.getNoteNumber());
        /* std::cout << "removing note " << msg.getNoteNumber() << "\n"; */
      }
    }

    midiMessages.clear();

    if (shouldTriggerNote()) {
      /* std::cout << "triggering " << "\n"; */
      if (currentlyPlayingNotes.size() > 0) {
        for (auto index = 0; index < currentlyPlayingNotes.size(); index++) {
          auto currentlyPlayingNote = currentlyPlayingNotes[index];
          midiMessages.addEvent(MidiMessage::noteOff(1, currentlyPlayingNote), 0);
        }
        currentlyPlayingNotes.clear();
      }

      if (notes.size() > 0) {
        for (auto index = 0; index < notes.size(); index++) {
          auto note = notes[index];
          /* std::cout << "playing note " << note << "\n"; */
          midiMessages.addEvent(MidiMessage::noteOn(1, note, (uint8) 85), 0);
          currentlyPlayingNotes.add(note);
        }
      }
    }
}

int RepeaterPluginAudioProcessor::getPlacementInBar(double currentPosition) {
  auto beatsPerNote = 4.0f / (float) notesPerBar;
  return std::floor(currentPosition / beatsPerNote);
}

int RepeaterPluginAudioProcessor::getMultiplePlacementInBar(double currentPosition) {
  if (multipleNext == 0) return -1;
  auto beatsPerNote = 4.0f / (float) (notesPerBar * multipleNext);
  return std::floor(currentPosition / beatsPerNote);
}

bool RepeaterPluginAudioProcessor::getRandomBoolByPercentage(int percentage) {
  return Random::getSystemRandom().nextInt(Range<int>(-percentage, 100 - percentage)) < 0;
}

int RepeaterPluginAudioProcessor::shouldMultipleNext() {
  std::map<int, int>::iterator it = multiplePercentages.begin();
  while (it != multiplePercentages.end()) {
    if (getRandomBoolByPercentage(it->second)) return it->first;
    it++;
  }
  return 0;
}

bool RepeaterPluginAudioProcessor::shouldSkipNext() {
  return getRandomBoolByPercentage(skipPercentage);
}

void RepeaterPluginAudioProcessor::updatePerBeat() {
  skipNext = shouldSkipNext();
  multipleNext = shouldMultipleNext();
}

bool RepeaterPluginAudioProcessor::shouldTriggerNote() {
  auto playHead = getPlayHead();
  AudioPlayHead::CurrentPositionInfo currentPosition;
  playHead->getCurrentPosition(currentPosition);
  auto barPosition = currentPosition.ppqPosition;
  auto currentPlacement = getPlacementInBar(barPosition);
  auto shouldTriggerBasedOnPosition = (barPosition < prevBarPosition || currentPlacement > getPlacementInBar(prevBarPosition));
  /* if (shouldTrigger) { */
  /*   std::cout << "currentPlacement " << currentPlacement << "\n"; */
  /*   std::cout << "prevPlacement " << getPlacementInBar(prevBarPosition) << "\n"; */
  /* } */
  auto prevMultiplePlacement = getMultiplePlacementInBar(prevBarPosition);
  prevBarPosition = barPosition;
  /* std::cout << "shouldTriggerBasedOnPosition " << shouldTriggerBasedOnPosition << "\n"; */
  if (!shouldTriggerBasedOnPosition) {
    if (!(multipleNext > 0)) return false;
    auto currentMultiplePlacement = getMultiplePlacementInBar(barPosition);
    auto shouldTriggerMultipleBasedOnPosition = currentMultiplePlacement > prevMultiplePlacement;
    if (shouldTriggerMultipleBasedOnPosition) return true;
    return false;
  }
  auto shouldTrigger = true;
  if (skipNext) shouldTrigger = false;
  updatePerBeat();
  return shouldTrigger;
}

//==============================================================================
bool RepeaterPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* RepeaterPluginAudioProcessor::createEditor()
{
    return new RepeaterPluginAudioProcessorEditor (*this);
}

//==============================================================================
void RepeaterPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void RepeaterPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RepeaterPluginAudioProcessor();
}
