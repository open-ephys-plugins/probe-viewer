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

namespace ProbeViewer
{

/**
    Custom button to show/hide average view
*/
class ShowAverageViewButton : public TextButton
{
public:
    /** Constructor */
    ShowAverageViewButton() : TextButton ("SHOW AVG") {}

    /** Destructor */
    virtual ~ShowAverageViewButton() {}

    /** Override paint method */
    void paint (Graphics& g) override;
};

/**
    Holds settings interface for the ProbeViewerCanvas
*/
class CanvasOptionsBar : public Component, public ComboBox::Listener, public Button::Listener
{
public:
    /** Constructor */
    CanvasOptionsBar (class ChannelViewCanvas* channelsView,
                      class ProbeViewerTimeScale* timeScale);

    /** Destructor */
    virtual ~CanvasOptionsBar() override;

    /** Draw the optiosn bar */
    void paint (Graphics& g) override;

    /** Called on resize */
    void resized() override;

    /** ComboBox callback */
    void comboBoxChanged (ComboBox* cb) override;

    /** Button callback*/
    void buttonClicked (Button* button) override;

    /** Sets the left margin offset in pixels of the render mode sub options panels */
    void setMarginOffset (float marginOffset);

    // ACCESSORS FOR OPTIONS BAR SUB-OPTIONS

    /** Return the RMS low bound for plotter color mapping */
    float getRMSLowBound() const;

    /** Return the RMS high bound for plotter color mapping */
    float getRMSHiBound() const;

    /** Return the difference between high and low bounds for RMS */
    float getRMSBoundSpread() const;

    /** Return the FFT low bound for plotter color mapping */
    float getFFTLowBound() const;

    /** Return the FFT high bound for plotter color mapping */
    float getFFTHiBound() const;

    /** Return the difference between high and low bounds for FFT */
    float getFFTBoundSpread() const;

    /** Return the selected center frequency bin for the FFT renderer */
    int getFFTCenterFrequencyBin() const;

    /**
     *  Recalculate the number of real-numbered output bins and their
     *  frequency mappings.
     *
     *  @param numBins      the number of FFT output bins
     *  @param sampleRate   the sampleRate of the input signal (for the
     *                      channels that are currently displayed)
     */
    void setFFTParams (const int numBins, const float sampleRate);

    /** Return the spike rate low bound for plotter color mapping */
    float getSpikeRateLowBound() const;

    /** Return the spike rate high bound for plotter color mapping */
    float getSpikeRateHiBound() const;

    /** Return the absolute difference between high and low bounds for spike rate */
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

    /**  Saves current parameters to XML */
    void saveParameters (XmlElement* xml);

    /** Loads stored parameters from XML */
    void loadParameters (XmlElement* xml);

    /**  Adds a listener to sub-components */
    void addListener (ComboBox::Listener* listener);

    ScopedPointer<ComboBox> renderModeSelection;

private:
    class ChannelViewCanvas* channelsView;
    class ProbeViewerTimeScale* timeScale;

    float marginWidth;

    Font labelFont;
    Colour labelColour;

    ScopedPointer<Label> renderModeSelectionLabel;

    ScopedPointer<ShowAverageViewButton> showAverageViewButton;

    ScopedPointer<Label> colourSchemeSelectionLabel;
    ScopedPointer<ComboBox> colourSchemeSelection;

    Component* currentSubOptionComponent;

    ScopedPointer<class RMSSubOptionComponent> rmsSubOptionComponent;
    ScopedPointer<class FFTSubOptionComponent> fftSubOptionComponent;
    ScopedPointer<class SpikeRateSubOptionComponent> spikeRateSubOptionComponent;

    bool displayLineSelectionInstructions = true;
};

/** 

    Displays options for RMS rendering mode

*/
class RMSSubOptionComponent : public Component, public ComboBox::Listener
{
public:
    /** Constructor */
    RMSSubOptionComponent (Font labelFont, Colour labelColour);

    /** Destructor */
    virtual ~RMSSubOptionComponent() override;

    /** Paint background */
    void paint (Graphics& g) override;

    /** Change width */
    void resized() override;

    /** Register listener to relevant ComboBoxes */
    void addListener (ComboBox::Listener* listener);

    /** ComboBox callback*/
    void comboBoxChanged (ComboBox* cb) override;

    /** Return the RMS low bound for plotter color mapping */
    float getRMSLowBound() const;

    /** Return the RMS high bound for plotter color mapping */
    float getRMSHiBound() const;

    /** Return the difference between high and low bounds for RMS */
    float getRMSBoundSpread() const;

    /** Sets the RMS Low and High bound values */
    void setRMSBounds (String low, String high);

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

/**

    Displays options for FFT rendering mode

*/
class FFTSubOptionComponent : public Component, public ComboBox::Listener
{
public:
    /** Constructor */
    FFTSubOptionComponent (Font labelFont, Colour labelColour);

    /** Destructor */
    virtual ~FFTSubOptionComponent() override;

    /** Paint background */
    void paint (Graphics& g) override;

    /** Change width */
    void resized() override;

    /** Register listener to relevant ComboBoxes */
    void addListener (ComboBox::Listener* listener);

    /** ComboBox callback*/
    void comboBoxChanged (ComboBox* cb) override;

    /** Change sample rate of input stream */
    void setSampleRate (const float sampleRate);

    /** Set number of FFT bins */
    void setFFTSize (const int numBins);

    /** Return the FFT low bound for plotter color mapping */
    float getFFTLowBound() const;

    /** Return the FFT high bound for plotter color mapping */
    float getFFTHiBound() const;

    /** Return the difference between high and low bounds for FFT */
    float getFFTBoundSpread() const;

    /** Return the selected center frequency bin for the FFT renderer */
    int getFFTSamplingBin() const;

    /** Return the selected center frequency bin for the FFT renderer */
    int getFFTFrequency() const;

    /** Sets the FFT parameters */
    void setFFTParams (String low, String high, String bin);

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

/**

    Displays options for spike rate rendering mode

*/
class SpikeRateSubOptionComponent : public Component, public ComboBox::Listener
{
public:
    /** Constructor */
    SpikeRateSubOptionComponent (Font labelFont, Colour labelColour);

    /** Destructor */
    virtual ~SpikeRateSubOptionComponent() override;

    /** Paint background */
    void paint (Graphics& g) override;

    /** Change width */
    void resized() override;

    /** Register listener to relevant ComboBoxes */
    void addListener (ComboBox::Listener* listener);

    /** ComboBox callback*/
    void comboBoxChanged (ComboBox* cb) override;

    /**  Return the spike rate low bound for plotter color mapping */
    float getSpikeRateLowBound() const;

    /** Return the spike rate high bound for plotter color mapping */
    float getSpikeRateHiBound() const;

    /** Return the absolute difference between high and low bounds for spike rate */
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

    /** Sets the Spike Rate parameters */
    void setSpikeRateParams (String low, String high, String threshold);

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

}; // namespace ProbeViewer

#endif /* CanvasOptionsBar_hpp */
