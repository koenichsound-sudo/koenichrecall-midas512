#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstdlib>

juce::NormalisableRange<float> recall::rangeFor(const Spec& s) {
 if (!s.logarithmic) return {s.min,s.max};
 return {s.min,s.max,
  [](float a,float b,float n) { return a * std::pow(b/a,n); },
  [](float a,float b,float v) { return std::log(v/a)/std::log(b/a); }};
}
juce::AudioProcessorValueTreeState::ParameterLayout RecallProcessor::makeLayout() {
 juce::AudioProcessorValueTreeState::ParameterLayout layout;
 for(const auto& s:recall::specs) {
  auto attrs=juce::AudioParameterFloatAttributes().withLabel(s.unit)
   .withStringFromValueFunction([s](float v,int) { return juce::String(v,s.logarithmic?1:(s.min<0?2:3)); })
   .withValueFromStringFunction([](const juce::String& text) { return text.replaceCharacter(',','.').getFloatValue(); });
  layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{s.id,1},s.name,recall::rangeFor(s),s.initial,attrs));
 }
 layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"treble_bell",1},"Treble BELL",false));
 layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bass_bell",1},"Bass BELL",false));
 layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"eq_in",1},"IN",true));
 return layout;
}
RecallProcessor::RecallProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 state(*this,nullptr,"KoenichMIDAS512State",makeLayout()) {}
bool RecallProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 return l.inputBuses.size()==1 && l.outputBuses.size()==1 && !l.getMainInputChannelSet().isDisabled()
  && l.getMainInputChannelSet()==l.getMainOutputChannelSet() && l.getMainInputChannelSet().size()<=64;
}
juce::AudioProcessorEditor* RecallProcessor::createEditor() { return new RecallEditor(*this); }
void RecallProcessor::getStateInformation(juce::MemoryBlock& data) {
 auto tree=state.copyState(); tree.setProperty("version",1,nullptr);
 if(auto xml=tree.createXml()) copyXmlToBinary(*xml,data);
}
void RecallProcessor::setStateInformation(const void* data,int bytes) {
 if(data==nullptr || bytes<=0) return;
 auto xml=getXmlFromBinary(data,bytes);
 if(!xml || !xml->hasTagName(state.state.getType())) return;
 auto incoming=juce::ValueTree::fromXml(*xml);
 if(!incoming.isValid() || (int)incoming.getProperty("version",1)!=1) return;
 // Validate the complete chunk before mutating anything. Unknown IDs, duplicate
 // values, NaN and out-of-range data must never poison a saved project.
 if(incoming.getNumChildren()!=15) return;
 juce::StringArray seen;
 for(auto child:incoming) {
  const auto id=child.getProperty("id").toString();
  auto* p=state.getParameter(id);
  if(!child.hasType("PARAM") || !child.hasProperty("value") || p==nullptr || seen.contains(id)) return;
  const auto value=child.getProperty("value");
  const auto valueText=value.toString();
  const auto* begin=valueText.toRawUTF8();char* end=nullptr;
  const float v=(float)std::strtod(begin,&end);
  if(end==begin || *end!='\0') return;
  const auto& r=p->getNormalisableRange();
  if(!std::isfinite(v) || v<r.start || v>r.end || (p->isBoolean() && v>0 && v<1)) return;
  seen.add(id);
 }
 state.replaceState(incoming);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RecallProcessor; }
