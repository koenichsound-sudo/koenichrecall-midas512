#pragma once
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class HardwareLook final : public juce::LookAndFeel_V4 {
public:
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
 void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
};
class RoundSlider final : public juce::Slider {
public:
 explicit RoundSlider(bool outer=false) : ring(outer) {}
 bool hitTest(int x,int y) override;
 void mouseDown(const juce::MouseEvent& e) override {
  setMouseDragSensitivity(e.mods.isShiftDown()?2500:250);
  juce::Slider::mouseDown(e);
 }
 double valueToProportionOfLength(double value) override {
  if (!width) return juce::Slider::valueToProportionOfLength(value);
  return widthRange().convertTo0to1(value);
 }
 double proportionOfLengthToValue(double proportion) override {
  if (!width) return juce::Slider::proportionOfLengthToValue(proportion);
  return widthRange().convertFrom0to1(proportion);
 }
 static juce::NormalisableRange<double> widthRange() {
  juce::NormalisableRange<double> range(0.1,2.0);
  range.setSkewForCentre(0.5);
  return range;
 }
 bool width=false;
 bool ring;
};
class RecallEditor final : public juce::AudioProcessorEditor {
public:
 explicit RecallEditor(RecallProcessor&);
 ~RecallEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 class Panel final : public juce::Component {
 public: void paint(juce::Graphics&) override;
 } panel;

 HardwareLook look;
 std::array<std::unique_ptr<RoundSlider>,12> knobs;
 std::array<std::unique_ptr<juce::Slider>,12> fields;
 std::array<std::unique_ptr<juce::ToggleButton>,3> buttons;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
 juce::TooltipWindow tips {this,500};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecallEditor)
};
