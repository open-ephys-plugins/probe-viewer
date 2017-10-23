//
//  CanvasOptionsBar.hpp
//  ProbeViewerPlugin
//
//  Created by Kelly Fox on 10/19/17.
//  Copyright © 2017 Allen Institute. All rights reserved.
//

#ifndef CanvasOptionsBar_hpp
#define CanvasOptionsBar_hpp

#include <VisualizerWindowHeaders.h>

namespace ProbeViewer {
    
class CanvasOptionsBar : public Component
    , public ComboBox::Listener
{
public:
    CanvasOptionsBar(class ChannelViewCanvas* channelsView);
    virtual ~CanvasOptionsBar() override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
    void comboBoxChanged(ComboBox * cb) override;
    
    void setMarginOffset(float marginOffset);
    
    // ACCESSORS FOR OPTIONS BAR SUB-OPTIONS
    float getRMSLowBound() const;
    float getRMSHiBound() const;
    float getRMSBoundSpread() const;
    
    float getFFTLowBound() const;
    float getFFTHiBound() const;
    float getFFTBoundSpread() const;
    
    float getSpikeRateLowBound() const;
    float getSpikeRateHiBound() const;
    float getSpikeRateBoundSpread() const;
    float getSpikeRateThreshold() const;
    
private:
    class ChannelViewCanvas* channelsView;
    
    float marginWidth;
    
    ColourGradient backgroundGradient;
    Colour foregroundColour;
    
    Font labelFont;
    Colour labelColour;
    
    ScopedPointer<Label> renderModeSelectionLabel;
    ScopedPointer<ComboBox> renderModeSelection;
    
    ScopedPointer<Label> colourSchemeSelectionLabel;
    ScopedPointer<ComboBox> colourSchemeSelection;
    
    Component* currentSubOptionComponent;
    
    ScopedPointer<class RMSSubOptionComponent> rmsSubOptionComponent;
    ScopedPointer<class FFTSubOptionComponent> fftSubOptionComponent;
    ScopedPointer<class SpikeRateSubOptionComponent> spikeRateSubOptionComponent;
};
    
class RMSSubOptionComponent : public Component
    , public ComboBox::Listener
{
public:
    RMSSubOptionComponent(Font labelFont, Colour labelColour);
    virtual ~RMSSubOptionComponent() override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
    void comboBoxChanged(ComboBox* cb) override;
    
    float getRMSLowBound() const;
    float getRMSHiBound() const;
    float getRMSBoundSpread() const;
    
private:
    Font labelFont;
    Colour labelColour;
    
    StringArray lowValueBoundSelectionOptions;
    ScopedPointer<Label> lowValueBoundLabel;
    ScopedPointer<ComboBox> lowValueBoundSelection;
    float lowValueBound;
    
    StringArray hiValueBoundSelectionOptions;
    ScopedPointer<Label> hiValueBoundLabel;
    ScopedPointer<ComboBox> hiValueBoundSelection;
    float hiValueBound;
};

class FFTSubOptionComponent : public Component
    , public ComboBox::Listener
{
public:
    FFTSubOptionComponent(Font labelFont, Colour labelColour);
    virtual ~FFTSubOptionComponent() override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
    void comboBoxChanged(ComboBox* cb) override;
    
    float getFFTLowBound() const;
    float getFFTHiBound() const;
    float getFFTBoundSpread() const;
    
private:
    Font labelFont;
    Colour labelColour;
    
    StringArray lowValueBoundSelectionOptions;
    ScopedPointer<Label> lowValueBoundLabel;
    ScopedPointer<ComboBox> lowValueBoundSelection;
    float lowValueBound;
    
    StringArray hiValueBoundSelectionOptions;
    ScopedPointer<Label> hiValueBoundLabel;
    ScopedPointer<ComboBox> hiValueBoundSelection;
    float hiValueBound;
};

class SpikeRateSubOptionComponent : public Component
    , public ComboBox::Listener
{
public:
    SpikeRateSubOptionComponent(Font labelFont, Colour labelColour);
    virtual ~SpikeRateSubOptionComponent() override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
    void comboBoxChanged(ComboBox* cb) override;
    
    float getSpikeRateLowBound() const;
    float getSpikeRateHiBound() const;
    float getSpikeRateBoundSpread() const;
    float getSpikeRateThreshold() const;
    
private:
    Font labelFont;
    Colour labelColour;
    
    StringArray lowValueBoundSelectionOptions;
    ScopedPointer<Label> lowValueBoundLabel;
    ScopedPointer<ComboBox> lowValueBoundSelection;
    float lowValueBound;
    
    StringArray hiValueBoundSelectionOptions;
    ScopedPointer<Label> hiValueBoundLabel;
    ScopedPointer<ComboBox> hiValueBoundSelection;
    float hiValueBound;
    
    StringArray thresholdSelectionOptions;
    ScopedPointer<Label> thresholdSelectionLabel;
    ScopedPointer<ComboBox> thresholdSelection;
    float threshold;
};
    
};

#endif /* CanvasOptionsBar_hpp */
