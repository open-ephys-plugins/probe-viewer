//
//  CanvasOptionsBar.hpp
//  ProbeViewerPlugin
//
//  Created by Kelly Fox on 10/19/17.
//  Copyright © 2017 Allen Institute. All rights reserved.
//

#ifndef CanvasOptionsBar_hpp
#define CanvasOptionsBar_hpp

#include "VisualizerWindowHeaders.h"

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

    /**
     * Sets the left margin offset in pixels of the render mode sub options panels
     */
    void setMarginOffset(float marginOffset);

    // ACCESSORS FOR OPTIONS BAR SUB-OPTIONS

    /**
     *  Return the RMS low bound for plotter color mapping
     */
    float getRMSLowBound() const;

    /**
     *  Return the RMS high bound for plotter color mapping
     */
    float getRMSHiBound() const;

    /**
     *  Return the difference between high and low bounds for RMS
     */
    float getRMSBoundSpread() const;

    /**
     *  Return the FFT low bound for plotter color mapping
     */
    float getFFTLowBound() const;

    /**
     *  Return the FFT high bound for plotter color mapping
     */
    float getFFTHiBound() const;

    /**
     *  Return the difference between high and low bounds for FFT
     */
    float getFFTBoundSpread() const;

    /**
     *  Return the selected center frequency bin for the FFT renderer
     */
    int getFFTCenterFrequencyBin() const;

    /**
     *  Recalculate the number of real-numbered output bins and their
     *  frequency mappings.
     *
     *  @param numBins      the number of FFT output bins
     *  @param sampleRate   the sampleRate of the input signal (for the
     *                      channels that are currently displayed)
     */
    void setFFTParams(const int numBins, const float sampleRate);

    /**
     *  Return the spike rate low bound for plotter color mapping
     */
    float getSpikeRateLowBound() const;

    /**
     *  Return the spike rate high bound for plotter color mapping
     */
    float getSpikeRateHiBound() const;

    /**
     *  Return the absolute difference between high and low bounds for
     *  spike rate
     */
    float getSpikeRateBoundSpread() const;

    /**
     *  Return the currently selected spike threshold.
     *
     *  This value describes the threshold below which a peak is
     *  considered a spike. It is intended to be used when input
     *  data is processed into a pixel, and describes the threshold
     *  after adjusting samples for median offset.
     */
    float getSpikeRateThreshold() const;

    void saveParameters(XmlElement* xml);

    void loadParameters(XmlElement* xml);

private:
    class ChannelViewCanvas* channelsView;

    float marginWidth;

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

    /**
     *  Return the RMS low bound for plotter color mapping
     */
    float getRMSLowBound() const;

    /**
     *  Return the RMS high bound for plotter color mapping
     */
    float getRMSHiBound() const;

    /**
     *  Return the difference between high and low bounds for RMS
     */
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

    void updateFrequencyRanges(const int numBins, const int sampleRate);
    void setSampleRate(const float sampleRate);
    void setFFTSize(const int numBins);

    /**
     *  Return the FFT low bound for plotter color mapping
     */
    float getFFTLowBound() const;

    /**
     *  Return the FFT high bound for plotter color mapping
     */
    float getFFTHiBound() const;

    /**
     *  Return the difference between high and low bounds for FFT
     */
    float getFFTBoundSpread() const;

    /**
     *  Return the selected center frequency bin for the FFT renderer
     */
    int getFFTSamplingBin() const;

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

    StringArray binSelectionOptions;
    ScopedPointer<Label> binSelectionLabel;
    ScopedPointer<ComboBox> binSelection;
    int binSelectionValue;

    float sampleRate;
    float maxFreq;
    int fftSize;
    int numBins;
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

    /**
     *  Return the spike rate low bound for plotter color mapping
     */
    float getSpikeRateLowBound() const;

    /**
     *  Return the spike rate high bound for plotter color mapping
     */
    float getSpikeRateHiBound() const;

    /**
     *  Return the absolute difference between high and low bounds for
     *  spike rate
     */
    float getSpikeRateBoundSpread() const;

    /**
     *  Return the currently selected spike threshold.
     *
     *  This value describes the threshold below which a peak is
     *  considered a spike. It is intended to be used when input
     *  data is processed into a pixel, and describes the threshold
     *  after adjusting samples for median offset.
     */
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
