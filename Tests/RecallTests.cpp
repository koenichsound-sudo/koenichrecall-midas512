#include "../Source/PluginEditor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <set>

static void check(bool v,const char* message) { if(!v) throw std::runtime_error(message); }
template<class T> static void audio(juce::AudioProcessor& p,int channels,int n) {
 juce::AudioBuffer<T> b(channels,n),copy(channels,n);
 for(int c=0;c<channels;++c) for(int i=0;i<n;++i) b.setSample(c,i,(T)(std::sin(i*0.129+c)*0.99));
 if(n>3) {b.setSample(0,0,(T)-0.0);b.setSample(0,1,std::numeric_limits<T>::denorm_min());b.setSample(0,2,std::numeric_limits<T>::infinity());}
 copy.makeCopyOf(b);juce::MidiBuffer midi;p.processBlock(b,midi);
 for(int c=0;c<channels;++c) check(std::memcmp(b.getReadPointer(c),copy.getReadPointer(c),(size_t)n*sizeof(T))==0,"Audio is not bit identical");
}
static void recallCheck(juce::AudioProcessor& a,juce::AudioProcessor& b) {
 check(a.getParameters().size()==15 && b.getParameters().size()==15,"15 parameters required");
 for(int pass=0;pass<3;++pass) {
  for(auto* p:a.getParameters()) p->setValueNotifyingHost(pass==0?0.0f:pass==1?1.0f:p->isBoolean()?(float)(p->getParameterIndex()%2):(float)(p->getParameterIndex()+1)/17.0f);
  juce::MemoryBlock block;a.getStateInformation(block);check(block.getSize()>0,"Empty state");b.setStateInformation(block.getData(),(int)block.getSize());
  for(int i=0;i<15;++i) {
   if(std::abs(a.getParameters()[i]->getValue()-b.getParameters()[i]->getValue())>=1e-5f) {
    std::cerr<<"Recall mismatch pass="<<pass<<" index="<<i<<" source="<<a.getParameters()[i]->getValue()<<" restored="<<b.getParameters()[i]->getValue()<<"\n";
    if(auto xml=juce::AudioProcessor::getXmlFromBinary(block.getData(),(int)block.getSize())) std::cerr<<xml->toString()<<"\n";
    check(false,"Recall round-trip failed");
   }
  }
 }
}
static juce::Component* find(juce::Component& root,const juce::String& id) {
 if(root.getComponentID()==id) return &root;
 for(auto* c:root.getChildren()) if(auto* match=find(*c,id)) return match;
 return nullptr;
}
int main(int argc,char** argv) {
 juce::ScopedJuceInitialiser_GUI init;
 try {
  RecallProcessor a,b;recallCheck(a,b);
  std::set<juce::String> ids;
  for(auto& s:recall::specs) {
   auto* p=a.state.getParameter(s.id);check(p!=nullptr,"Missing parameter");ids.insert(s.id);
   check(std::abs(p->convertFrom0to1(0)-s.min)<0.001f && std::abs(p->convertFrom0to1(1)-s.max)<0.01f,"Range endpoints");
   if(s.logarithmic) check(std::abs(p->convertFrom0to1(0.5f)-std::sqrt(s.min*s.max))<0.01f,"Frequency is not logarithmic");
   for(int i=0;i<=100;++i) check(std::abs(p->convertTo0to1(p->convertFrom0to1((float)i/100))-(float)i/100)<1e-5f,"Mapping round-trip");
  }
  check(ids.size()==12,"Duplicate IDs");
  std::array<float,15> beforeInvalid{};
  for(int i=0;i<15;++i) beforeInvalid[(size_t)i]=b.getParameters()[i]->getValue();
  auto unchanged=[&] {for(int i=0;i<15;++i) check(beforeInvalid[(size_t)i]==b.getParameters()[i]->getValue(),"Invalid state changed parameters");};
  b.setStateInformation("bad",3);unchanged();
  for(const auto& invalid: {juce::String("nan"),juce::String("oops"),juce::String("1000000")}) {
   auto tree=a.state.copyState();tree.getChild(0).setProperty("value",invalid,nullptr);
   auto xml=tree.createXml();juce::MemoryBlock block;juce::AudioProcessor::copyXmlToBinary(*xml,block);b.setStateInformation(block.getData(),(int)block.getSize());unchanged();
  }
  for(int channels:{1,2,8,64}) {
   auto set=channels==1?juce::AudioChannelSet::mono():channels==2?juce::AudioChannelSet::stereo():juce::AudioChannelSet::discreteChannels(channels);
   juce::AudioProcessor::BusesLayout l;l.inputBuses.add(set);l.outputBuses.add(set);check(a.setBusesLayout(l),"Matching bus layout rejected");
   for(double rate:{44100.0,48000.0,96000.0}) {
    a.prepareToPlay(rate,1024);
    for(int in:{0,1}) {a.state.getParameter("eq_in")->setValueNotifyingHost((float)in);for(int n:{0,1,64,1024}) {audio<float>(a,channels,n);audio<double>(a,channels,n);}}
    a.releaseResources();
   }
  }
  juce::AudioProcessor::BusesLayout wrong;wrong.inputBuses.add(juce::AudioChannelSet::mono());wrong.outputBuses.add(juce::AudioChannelSet::stereo());check(!a.isBusesLayoutSupported(wrong),"Mismatched buses accepted");
  {
   std::unique_ptr<juce::AudioProcessorEditor> e(b.createEditor());
   for(size_t i=0;i<12;++i) {
    const auto& s=recall::specs[i];auto* k=dynamic_cast<RoundSlider*>(find(*e,s.id));auto* f=dynamic_cast<juce::Slider*>(find(*e,juce::String(s.id)+"_value"));
    check(k && f,"Missing knob or value field");
    if(i%3==2) {
     check(std::abs(k->valueToProportionOfLength(0.5)-0.5)<1e-6,"Width 0.5 must be at noon");
     check(std::abs(k->proportionOfLengthToValue(0.5)-0.5)<1e-6,"Width noon must equal 0.5");
     for(int step=0;step<=100;++step) {double n=step/100.0;check(std::abs(k->valueToProportionOfLength(k->proportionOfLengthToValue(n))-n)<1e-6,"Width GUI mapping round-trip");}
    }
    k->setValue(s.max,juce::sendNotificationSync);check(std::abs(b.state.getRawParameterValue(s.id)->load()-s.max)<0.01f,"Knob to parameter failed");
    check(std::abs(f->getValue()-s.max)<0.01,"Knob to field failed");
    f->setValue(s.min,juce::sendNotificationSync);check(std::abs(k->getValue()-s.min)<0.01,"Field to knob failed");
    b.state.getParameter(s.id)->setValueNotifyingHost(0.5f);check(std::abs(k->getValue()-b.state.getParameter(s.id)->convertFrom0to1(0.5f))<0.01,"Host to GUI failed");
   }
   for(int band=0;band<4;++band) {
    auto* gain=dynamic_cast<RoundSlider*>(find(*e,recall::specs[(size_t)band*3].id));auto* freq=dynamic_cast<RoundSlider*>(find(*e,recall::specs[(size_t)band*3+1].id));
    auto* panel=gain->getParentComponent();
    check(panel->getComponentAt(gain->getBounds().getCentre())==gain,"Centre must target gain");
    check(panel->getComponentAt(freq->getX()+5,freq->getY()+freq->getHeight()/2)==freq,"Outer ring must target frequency");
    check(!gain->hitTest(0,0) && !freq->hitTest(0,0),"Rectangular hit area leaks");
    auto oldFreq=freq->getValue();gain->setValue(12,juce::sendNotificationSync);check(freq->getValue()==oldFreq,"Gain changed frequency");
    auto oldGain=gain->getValue();freq->setValue(freq->getMaximum(),juce::sendNotificationSync);check(gain->getValue()==oldGain,"Frequency changed gain");
   }
   for(auto id:recall::toggleIDs) {
    auto* t=dynamic_cast<juce::ToggleButton*>(find(*e,id));check(t!=nullptr,"Missing toggle");
    if(juce::String(id).contains("bell")) check(t->getTooltip().startsWith(juce::String::fromUTF8("Grün")),"UTF-8 BELL tooltip");
    for(bool on:{false,true}) {t->setToggleState(on,juce::sendNotificationSync);check(b.state.getRawParameterValue(id)->load()==(on?1:0),"Toggle to parameter");}
   }
   // Restore while editor is open and ensure every attached control updates.
   recallCheck(a,b);
   for(auto& s:recall::specs) {auto* k=dynamic_cast<RoundSlider*>(find(*e,s.id));check(std::abs(k->getValue()-b.state.getRawParameterValue(s.id)->load())<0.01,"Open editor recall");}
  }
  if(argc>1) {
   RecallProcessor defaults;std::unique_ptr<juce::AudioProcessorEditor> e(defaults.createEditor());
   juce::FileOutputStream out{juce::File(argv[1])};check(out.openedOk(),"Cannot write preview");
   out.setPosition(0);check(out.truncate().wasOk(),"Cannot replace preview");
   juce::PNGImageFormat png;check(png.writeImageToStream(e->createComponentSnapshot(e->getLocalBounds(),true,2),out),"PNG encoding failed");
  }
  std::cout<<"PASS: parameter ranges, logarithmic mappings, 15-value recall, malformed-state rejection, GUI/host synchronization, concentric hit areas and independence, float/double bit-exact audio (1/2/8/64 channels, 44.1/48/96 kHz, IN on/off).\n";
  if(argc>2) {
   juce::AudioPluginFormatManager manager;manager.addFormat(std::make_unique<juce::VST3PluginFormat>());
   juce::OwnedArray<juce::PluginDescription> types;manager.getFormat(0)->findAllTypesForFile(types,argv[2]);check(types.size()==1,"VST3 scan failed");
   juce::String error;auto one=manager.createPluginInstance(*types[0],48000,512,error);check(one!=nullptr,error.toRawUTF8());
   auto two=manager.createPluginInstance(*types[0],48000,512,error);check(two!=nullptr,error.toRawUTF8());
   // JUCE's VST3 wrapper may expose its own bypass parameter in addition to 15 recall values.
   check(one->getParameters().size()>=15,"VST3 parameter exposure failed");
   // VST3 host adapters need not report isBoolean for two-step parameters.
   for(auto* p:one->getParameters()) p->setValueNotifyingHost(p->getParameterIndex()>=12?0.0f:0.73f);
   one->prepareToPlay(48000,512);audio<float>(*one,2,512);one->releaseResources();
   juce::MemoryBlock block;one->getStateInformation(block);two->setStateInformation(block.getData(),(int)block.getSize());
   for(int i=0;i<15;++i) {
    if(std::abs(one->getParameters()[i]->getValue()-two->getParameters()[i]->getValue())>=1e-5f) {
     std::cerr<<"Hosted mismatch "<<i<<" "<<one->getParameters()[i]->getName(100)<<": "<<one->getParameters()[i]->getValue()<<" vs "<<two->getParameters()[i]->getValue()<<"\n";
     check(false,"Hosted VST3 recall failed");
    }
   }
   std::unique_ptr<juce::AudioProcessorEditor> editor(two->createEditorAndMakeActive());check(editor!=nullptr,"Hosted VST3 editor creation failed");
   std::cout<<"PASS: compiled ARM64 VST3 scan, two instances, state round-trip, audio passthrough and native editor creation.\n";
  }
  return 0;
 } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
