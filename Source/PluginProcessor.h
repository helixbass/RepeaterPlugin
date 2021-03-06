/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <map>

//==============================================================================
/**
*/
class RepeaterPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    RepeaterPluginAudioProcessor();
    ~RepeaterPluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepeaterPluginAudioProcessor)
    bool shouldTriggerNote();
    int getPlacementInBar(double barPosition);
    bool shouldSkipNext();
    bool getRandomBoolByPercentage(int percentage);
    void updatePerBeat();
    int shouldMultipleNext();
    int getMultiplePlacementInBar(double barPosition);

    SortedSet<int> notes;
    SortedSet<int> currentlyPlayingNotes;
    double prevBarPosition;
    int notesPerBar;
    int skipPercentage;
    bool skipNext;
    std::map<int, int> multiplePercentages;
    int multipleNext;
};
