#include "PluginEditor.h"
#include <cmath>

namespace {
constexpr float pi=juce::MathConstants<float>::pi;
constexpr float start=7*pi/6, finish=17*pi/6; // 7 o'clock to 5 o'clock, clockwise.
const juce::Colour blue(0xff009bd6), ink(0xffc8d9df);
juce::Point<float> polar(float x,float y,float r,float angle) {
 return {x+r*std::sin(angle),y-r*std::cos(angle)};
}
void text(juce::Graphics& g,const juce::String& s,juce::Rectangle<float> r,float size,juce::Colour c=ink) {
 g.setColour(c);g.setFont(juce::FontOptions(size));g.drawText(s,r,juce::Justification::centred);
}
void text(juce::Graphics& g,const char* s,juce::Rectangle<float> r,float size,juce::Colour c=ink) {
 text(g,juce::String::fromUTF8(s),r,size,c);
}
void scale(juce::Graphics& g,float x,float y,float r,float low,float high,bool logarithmic) {
 for(int i=0;i<=10;++i) {
  const float a=start+(finish-start)*(float)i/10;
  const auto p=polar(x,y,r,a),q=polar(x,y,r+(i%5==0?15.0f:9.0f),a);
  g.setColour(ink.withAlpha(i%5==0?0.9f:0.65f));g.drawLine({p,q},i%5==0?6.0f:3.0f);
 }
 auto label=[](float v) { return v>=1000?juce::String(juce::roundToInt(v/1000))+"k":v<2?juce::String(v,1):juce::String(juce::roundToInt(v)); };
 std::array<float,4> marks {low,0.3f,0.6f,high};
 if(logarithmic) {
  if(low>=1000) marks={low,3000,8000,high};
  else if(low>=400) marks={low,1000,3000,high};
  else if(low>=100) marks={low,300,800,high};
  else marks={low,50,100,high};
 }
 for(float v:marks) {
  float n=logarithmic?std::log(v/low)/std::log(high/low):(float)RoundSlider::widthRange().convertTo0to1(v);
  auto p=polar(x,y,r+(logarithmic?36.0f:18.0f),start+n*(finish-start));
  if(!logarithmic && n<0.01f) p.x+=20; // Separate from the following band's frequency scale.
  text(g,label(v),{p.x-34,p.y-13,68,26},20);
 }
}
}

bool RoundSlider::hitTest(int x,int y) {
 const float distance=juce::Point<float>((float)x,(float)y).getDistanceFrom(getLocalBounds().toFloat().getCentre());
 const float radius=(float)juce::jmin(getWidth(),getHeight())*0.5f;
 return distance<=radius && (!ring || distance>=radius*0.69f);
}
void HardwareLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float a,float b,juce::Slider& slider) {
 const bool ring=slider.getProperties().getWithDefault("ring",false);
 auto bounds=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(2);
 const float r=juce::jmin(bounds.getWidth(),bounds.getHeight())*0.5f;
 const auto centre=bounds.getCentre();
 bounds={centre.x-r,centre.y-r,2*r,2*r};
 g.setColour(juce::Colours::black.withAlpha(0.5f));g.fillEllipse(bounds.translated(0,3));
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff737d82),bounds.getTopLeft(),juce::Colour(0xff11191e),bounds.getBottomRight(),false));g.fillEllipse(bounds);
 g.setColour(juce::Colour(0xff899398));g.drawEllipse(bounds.reduced(1),1);
 for(int i=0;i<24;++i) {
  float angle=2*pi*(float)i/24;
  auto p=polar(centre.x,centre.y,r*0.85f,angle),q=polar(centre.x,centre.y,r*0.97f,angle);
  g.setColour(juce::Colours::black.withAlpha(0.32f));g.drawLine({p,q},r*0.055f);
 }
 auto face=bounds.reduced(r*(ring?0.18f:0.22f));
 const auto colour=slider.findColour(juce::Slider::rotarySliderFillColourId);
 g.setGradientFill(juce::ColourGradient(colour.brighter(0.3f),face.getTopLeft(),colour.darker(0.18f),face.getBottomRight(),false));g.fillEllipse(face);
 g.setColour(juce::Colours::black.withAlpha(0.65f));g.drawEllipse(face,1.2f);
 const float angle=a+position*(b-a);
 auto p=polar(centre.x,centre.y,r*(ring?0.78f:0.12f),angle),q=polar(centre.x,centre.y,r*0.95f,angle);
 g.setColour(juce::Colours::black.withAlpha(0.8f));g.drawLine({p.translated(1,1),q.translated(1,1)},r*0.16f);
 g.setColour(ring?juce::Colour(0xffffe3a5):juce::Colours::white);g.drawLine({p,q},r*(ring?0.1f:0.115f));
 if(slider.hasKeyboardFocus(false)) {g.setColour(blue);g.drawEllipse(bounds.reduced(1),2);}
}
void HardwareLook::drawToggleButton(juce::Graphics& g,juce::ToggleButton& button,bool over,bool down) {
 auto r=button.getLocalBounds().toFloat().reduced(2);
 g.setColour(juce::Colour(0xff050a0e));g.fillRoundedRectangle(r,7);
 auto face=r.reduced(5);
 g.setGradientFill(juce::ColourGradient(juce::Colour(down?0xffaaaaaa:0xfff3f3ef),face.getTopLeft(),juce::Colour(0xff999fa2),face.getBottomRight(),false));g.fillRoundedRectangle(face,4);
 auto led=face.removeFromRight(face.getWidth()*0.2f).reduced(2,face.getHeight()*0.18f);
 const auto c=button.findColour(juce::ToggleButton::tickColourId);
 g.setColour(button.getToggleState()?c:juce::Colour(0xff333c40));g.fillRoundedRectangle(led,3);
 if(button.getToggleState()) {g.setColour(c.withAlpha(0.22f));g.fillRoundedRectangle(led.expanded(3),5);}
 text(g,button.getButtonText(),face,juce::jmin(26.0f,face.getHeight()*0.48f),juce::Colour(0xff171b1e));
 if(over || button.hasKeyboardFocus(false)) {g.setColour(blue);g.drawRoundedRectangle(r,7,2);}
}
void RecallEditor::Panel::paint(juce::Graphics& g) {
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff006aab),0,0,juce::Colour(0xff003c7f),521,1816,false));g.fillAll();
 for(int x=5;x<521;x+=5) {g.setColour(juce::Colours::white.withAlpha(x%3==0?0.035f:0.012f));g.drawVerticalLine(x,0,1816);}
 g.setColour(juce::Colour(0xff00b8dc));g.drawRect(1,1,519,1814,5);
 g.setColour(ink);g.drawRoundedRectangle(20,105,480,1600,43,3);
 text(g,"MIDAS",{25,24,177,65},49,juce::Colour(0xfff4ebbd));
 text(g,"512",{353,24,139,65},49,juce::Colour(0xfff4ebbd));
 for(float y:{46.0f,1761.0f}) {g.setColour(juce::Colour(0xff082c46));g.fillEllipse(223,y-36,74,74);g.setColour(juce::Colour(0xff616a70));g.fillEllipse(240,y-20,40,40);g.setColour(juce::Colour(0xff252d34));g.drawLine(245,y,275,y,4);}
 const char* names[]{"treble","hi mid","lo mid","bass"};
 for(int band=0;band<4;++band) {
  const float cy=240.0f+(float)band*383;
  scale(g,205,cy,112,recall::specs[(size_t)band*3+1].min,recall::specs[(size_t)band*3+1].max,true);
  scale(g,399,cy+190,79,0.1f,2,false);
  {juce::Graphics::ScopedSaveState saved(g);g.addTransform(juce::AffineTransform::rotation(-pi/2,50,cy));text(g,names[band],{50-100,cy-20,200,40},35);}
  if(band<3) text(g,"gain / freq",{116,cy+151,178,36},25);
  text(g,"width",{326,cy+38,146,32},28);
 }
 text(g,"4 Band",{35,1718,173,78},43,juce::Colour(0xfff4ebbd));text(g,"PEQ",{356,1718,140,78},43,juce::Colour(0xfff4ebbd));
}
RecallEditor::RecallEditor(RecallProcessor& p) : AudioProcessorEditor(p) {
 setLookAndFeel(&look);addAndMakeVisible(panel);panel.setSize(521,1816);
 for(size_t i=0;i<12;++i) {
  const auto& spec=recall::specs[i];const auto kind=i%3;
  auto& knob=knobs[i];knob=std::make_unique<RoundSlider>(kind==1);
  knob->width=kind==2;
  knob->setComponentID(spec.id);knob->setName(spec.name);
  knob->setSliderStyle(juce::Slider::RotaryVerticalDrag);knob->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  knob->setRotaryParameters(start,finish,true);knob->setMouseDragSensitivity(250);
  knob->setDoubleClickReturnValue(true,spec.initial);knob->setScrollWheelEnabled(false);
  knob->setWantsKeyboardFocus(true);knob->getProperties().set("ring",kind==1);
  const size_t band=i/3;const auto colour=band==0?juce::Colour(0xffd9dcdd):(band==3?juce::Colour(0xff787e81):blue);
  knob->setColour(juce::Slider::rotarySliderFillColourId,kind==1?juce::Colour(0xff444c52):colour);
  knob->setTooltip(juce::String(spec.name)+" ("+spec.unit+juce::String::fromUTF8(") — ziehen; Shift = fein; Doppelklick = Standard"));
  panel.addAndMakeVisible(*knob);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,spec.id,*knob));
  auto& field=fields[i];field=std::make_unique<juce::Slider>(juce::Slider::LinearBar,juce::Slider::TextBoxLeft);
  field->setName(spec.name);field->setComponentID(juce::String(spec.id)+"_value");
  field->setTextBoxStyle(juce::Slider::TextBoxLeft,false,145,30);field->setTextValueSuffix(juce::String(" ")+spec.unit);
  field->setNumDecimalPlacesToDisplay(kind==1?1:3);field->setScrollWheelEnabled(false);
  field->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff142631));
  field->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffeaf3f6));
  field->setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff37505e));
  field->setColour(juce::Slider::trackColourId,juce::Colours::transparentBlack);
  field->setTooltip(juce::String(spec.name)+": Zahlenwert direkt eingeben");addAndMakeVisible(*field);
  attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,spec.id,*field));
 }
 // Put each smaller gain knob above its frequency ring. Circular hit-testing
 // gives the ring the surrounding annulus and captures the target during drag.
 for(size_t band=0;band<4;++band) knobs[band*3]->toFront(false);
 for(size_t i=0;i<3;++i) {
  buttons[i]=std::make_unique<juce::ToggleButton>(i==2?"IN":"BELL");
  buttons[i]->setName(i==0?"Treble BELL":i==1?"Bass BELL":"IN");buttons[i]->setComponentID(recall::toggleIDs[i]);
  buttons[i]->setColour(juce::ToggleButton::tickColourId,i==2?juce::Colour(0xffff971e):juce::Colour(0xff55ef63));
  buttons[i]->setTooltip(juce::String::fromUTF8(i==2?"Hardware IN: Recall-Zustand; Audio bleibt unverändert":"Grün = BELL; aus = Shelf. Width bleibt gespeichert."));
  panel.addAndMakeVisible(*buttons[i]);buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,recall::toggleIDs[i],*buttons[i]));
 }
 setResizable(true,true);setResizeLimits(560,820,980,1435);getConstrainer()->setFixedAspectRatio(640.0/940.0);setSize(640,940);
}
RecallEditor::~RecallEditor() {setLookAndFeel(nullptr);}
void RecallEditor::resized() {
 const float s=(float)getHeight()/940;
 panel.setTransform(juce::AffineTransform::scale((float)(getHeight()-40*s)/1816).translated(16*s,20*s));
 for(size_t band=0;band<4;++band) {
  const int cy=240+(int)band*383;
  knobs[band*3+1]->setBounds(102,cy-103,206,206);
  knobs[band*3]->setBounds(130,cy-75,150,150);
  knobs[band*3+2]->setBounds(328,cy+119,142,142);
  for(size_t k=0;k<3;++k) fields[band*3+k]->setBounds(juce::roundToInt(442*s),juce::roundToInt((202+(float)band*147+(float)k*34)*s),juce::roundToInt(165*s),juce::roundToInt(29*s));
 }
 buttons[0]->setBounds(337,190,140,90);buttons[1]->setBounds(337,1339,140,90);buttons[2]->setBounds(102,1532,143,92);
}
void RecallEditor::paint(juce::Graphics& g) {
 g.fillAll(juce::Colour(0xff0c1922));g.addTransform(juce::AffineTransform::scale((float)getHeight()/940));
 text(g,"KOENICH RECALL",{305,38,305,26},23,juce::Colour(0xffecf0f1));
 text(g,"MIDAS 512 EQ",{305,71,305,29},25,blue);
 text(g,"V1.0.1  /  HARDWARE SETTINGS",{305,107,305,22},12,juce::Colour(0xff8398a4));
 const char* names[]{"TREBLE","HI MID","LO MID","BASS"};
 for(int band=0;band<4;++band) {
  const float y=166.0f+(float)band*147;
  g.setColour(juce::Colour(0xff142631));g.fillRoundedRectangle(306,y,307,135,9);
  text(g,names[band],{319,y+8,120,22},15,blue);
  const char* labels[]{"Gain · innen","Freq · außen","Width"};
  for(int k=0;k<3;++k) text(g,juce::String::fromUTF8(labels[k]),{313,y+36+(float)k*34,121,29},13);
 }
 text(g,"Innen: Gain    ·    Außen: Frequenz",{301,781,315,24},14);
 text(g,"Ziehen  /  Shift: fein  /  Doppelklick: Reset",{298,811,323,24},12);
 text(g,"Zahlenfelder sind direkt editierbar.",{301,841,315,24},13);
 text(g,"RECALL ONLY · AUDIO UNVERÄNDERT",{301,888,315,24},12,juce::Colour(0xfff6c777));
}
