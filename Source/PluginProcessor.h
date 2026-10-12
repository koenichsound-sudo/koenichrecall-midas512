#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace recall {
struct Spec { const char* id; const char* name; float min, max, initial; const char* unit; bool logarithmic; };
inline constexpr std::array<Spec,12> specs {{
 {"treble_gain","Treble Gain",-15,15,0,"dB",false},
 {"treble_freq","Treble Frequency",1000,20000,4000,"Hz",true},
 {"treble_width","Treble Width",0.1f,2,0.5f,"oct",false},
 {"hi_mid_gain","Hi Mid Gain",-15,15,0,"dB",false},
 {"hi_mid_freq","Hi Mid Frequency",400,8000,1600,"Hz",true},
 {"hi_mid_width","Hi Mid Width",0.1f,2,0.5f,"oct",false},
 {"lo_mid_gain","Lo Mid Gain",-15,15,0,"dB",false},
 {"lo_mid_freq","Lo Mid Frequency",100,2000,400,"Hz",true},
 {"lo_mid_width","Lo Mid Width",0.1f,2,0.5f,"oct",false},
 {"bass_gain","Bass Gain",-15,15,0,"dB",false},
 {"bass_freq","Bass Frequency",20,400,80,"Hz",true},
 {"bass_width","Bass Width",0.1f,2,0.5f,"oct",false}
}};
inline constexpr std::array<const char*,3> toggleIDs {"treble_bell","bass_bell","eq_in"};
juce::NormalisableRange<float> rangeFor(const Spec&);
}

class RecallProcessor final : public juce::AudioProcessor {
public:
 RecallProcessor();
 juce::AudioProcessorValueTreeState state;
 const juce::String getName() const override { return "Koenich Recall - MIDAS 512 EQ"; }
 void prepareToPlay(double,int) override {}
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 // In-place identity: no sample writes, allocations, locks, or parameter reads.
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override {}
 void processBlock(juce::AudioBuffer<double>&,juce::MidiBuffer&) override {}
 bool supportsDoublePrecisionProcessing() const override { return true; }
 bool acceptsMidi() const override { return false; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 0; }
 bool hasEditor() const override { return true; }
 juce::AudioProcessorEditor* createEditor() override;
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return "Recall"; }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecallProcessor)
};
