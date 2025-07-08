//
//  CanvasOptionsBar.cpp
//  ProbeViewerPlugin
//
//  Created by Kelly Fox on 10/19/17.
//  Copyright © 2017 Allen Institute. All rights reserved.
//

#include "CanvasOptionsBar.hpp"

#include "../ProbeViewerCanvas.h"
#include "../TimeScale/ProbeViewerTimeScale.hpp"
#include "../Utilities/ColourScheme.hpp"
#include "ChannelViewCanvas.hpp"

using namespace ProbeViewer;

CanvasOptionsBar::CanvasOptionsBar (class ChannelViewCanvas* channelsView_,
                                    class ProbeViewerTimeScale* timeScale_)
    : channelsView (channelsView_), timeScale (timeScale_), marginWidth (0)
{
    labelFont = FontOptions ("Inter", "Regular", 16.0f);

    optionsViewport = std::make_unique<Viewport>();
    optionsViewport->setScrollBarsShown (false, true);
    optionsViewport->setScrollBarThickness (12);
    addAndMakeVisible (optionsViewport.get());

    optionsHolder = std::make_unique<Component> ("Main options");
    optionsViewport->setViewedComponent (optionsHolder.get(), false);

    rmsSubOptionComponent.reset (new RMSSubOptionComponent (labelFont));
    fftSubOptionComponent.reset (new FFTSubOptionComponent (labelFont));
    spikeRateSubOptionComponent.reset (new SpikeRateSubOptionComponent (labelFont));

    currentSubOptionComponent = rmsSubOptionComponent.get();
    optionsHolder->addAndMakeVisible (currentSubOptionComponent);

    minWidth = 730; // min width for RMS mode

    renderModeSelectionLabel.reset (new Label ("renderModeSelectionLabel", "Render Mode"));
    renderModeSelectionLabel->setFont (labelFont);
    optionsHolder->addAndMakeVisible (renderModeSelectionLabel.get());

    StringArray renderModeNames = { "RMS Signal", "Freq. Band Power", "Spike Rate" };
    renderModeSelection.reset (new ComboBox ("renderModeSelection"));
    renderModeSelection->addItemList (renderModeNames, 1);
    renderModeSelection->setEditableText (false);
    renderModeSelection->addListener (this);
    renderModeSelection->setSelectedId (1, dontSendNotification);
    optionsHolder->addAndMakeVisible (renderModeSelection.get());

    showAverageViewButton.reset (new ShowAverageViewButton());
    showAverageViewButton->setClickingTogglesState (true);
    showAverageViewButton->setToggleState (false, dontSendNotification);
    showAverageViewButton->addListener (this);
    optionsHolder->addAndMakeVisible (showAverageViewButton.get());

    // colour scheme options
    colourSchemeSelectionLabel.reset (new Label ("colourSchemeSelectionLabel", "Colour\nScheme"));
    colourSchemeSelectionLabel->setFont (labelFont);
    optionsHolder->addAndMakeVisible (colourSchemeSelectionLabel.get());

    StringArray colourSchemeNames = { "Inferno", "Plasma", "Magma", "Viridis", "Jet" };
    colourSchemeSelection.reset (new ComboBox ("colourSchemeSelection"));
    colourSchemeSelection->addItemList (colourSchemeNames, 1);
    colourSchemeSelection->setEditableText (false);
    colourSchemeSelection->addListener (this);
    colourSchemeSelection->setSelectedId (1, dontSendNotification);
    optionsHolder->addAndMakeVisible (colourSchemeSelection.get());

    setBufferedToImage (true);
}

CanvasOptionsBar::~CanvasOptionsBar()
{
}

void CanvasOptionsBar::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::componentBackground));
    g.fillRect (0, 0, getWidth(), getHeight());

    g.setColour (findColour (ThemeColours::componentBackground));
    g.drawLine (marginWidth, 0, marginWidth, getHeight(), 1);
}

void CanvasOptionsBar::addListener (ComboBox::Listener* listener)
{
    renderModeSelection->addListener (listener);
    colourSchemeSelection->addListener (listener);
    rmsSubOptionComponent->addListener (listener);
    fftSubOptionComponent->addListener (listener);
    spikeRateSubOptionComponent->addListener (listener);
}

void CanvasOptionsBar::resized()
{
    optionsViewport->setBounds (0, 0, getWidth(), getHeight());

    int optionsWidth = getWidth() < minWidth ? minWidth : getWidth();

    optionsHolder->setBounds (0, 0, optionsWidth, 30);

    renderModeSelectionLabel->setBounds (0, 0, 95, optionsHolder->getHeight());
    renderModeSelection->setBounds (renderModeSelectionLabel->getRight(), 4, 100, optionsHolder->getHeight() - 8);

    showAverageViewButton->setBounds (optionsHolder->getRight() - 50, 0, 45, optionsHolder->getHeight());

    //int colourSchemeOffset = 700;
    //if (getWidth() > colourSchemeOffset) colourSchemeOffset = getWidth();
    colourSchemeSelectionLabel->setBounds (optionsHolder->getRight() - 220, 0, 70, optionsHolder->getHeight());
    colourSchemeSelection->setBounds (colourSchemeSelectionLabel->getRight(), 4, 90, optionsHolder->getHeight() - 8);

    Rectangle<int> subOptionBounds (marginWidth + 3, 0, optionsWidth - marginWidth - 220 - 3, optionsHolder->getHeight());
    rmsSubOptionComponent->setBounds (subOptionBounds);
    fftSubOptionComponent->setBounds (subOptionBounds);
    spikeRateSubOptionComponent->setBounds (subOptionBounds);
}

void CanvasOptionsBar::comboBoxChanged (ComboBox* cb)
{
    if (cb == renderModeSelection.get())
    {
        optionsHolder->removeChildComponent (currentSubOptionComponent);

        RenderMode renderMode;

        switch (cb->getSelectedId())
        {
            case 1:
                renderMode = RenderMode::RMS;
                currentSubOptionComponent = rmsSubOptionComponent.get();
                minWidth = 730;
                break;

            case 2:
                renderMode = RenderMode::FFT;
                currentSubOptionComponent = fftSubOptionComponent.get();
                minWidth = 980;
                break;

            case 3:
            default:
                renderMode = RenderMode::SPIKE_RATE;
                currentSubOptionComponent = spikeRateSubOptionComponent.get();
                minWidth = 980;
                break;
        }

        optionsHolder->addAndMakeVisible (currentSubOptionComponent);

        resized();

        channelsView->setCurrentRenderMode (renderMode);
    }
    else if (cb == colourSchemeSelection.get())
    {
        ColourSchemeId colourSchemeId;

        switch (cb->getSelectedId())
        {
            case 1:
                colourSchemeId = ColourSchemeId::INFERNO;
                break;

            case 2:
                colourSchemeId = ColourSchemeId::PLASMA;
                break;

            case 3:
                colourSchemeId = ColourSchemeId::MAGMA;
                break;

            case 4:
                colourSchemeId = ColourSchemeId::VIRIDIS;
                break;

            case 5:
            default:
                colourSchemeId = ColourSchemeId::JET;
                break;
        }

        channelsView->setCurrentColourScheme (colourSchemeId);
    }
}

void CanvasOptionsBar::buttonClicked (Button* button)
{
    if (button == showAverageViewButton.get())
    {
        channelsView->showAverageView (showAverageViewButton->getToggleState());
        timeScale->showAverageView (showAverageViewButton->getToggleState());
    }
}

void CanvasOptionsBar::setMarginOffset (float marginOffset)
{
    marginWidth = marginOffset;
    resized();
}

void ShowAverageViewButton::paint (Graphics& g)
{
    g.setFont (FontOptions ("Inter", "Regular", 12.0f));

    if (getToggleState())
    {
        g.setColour (findColour (ThemeColours::defaultText));
        g.drawText ("HIDE", 0, 3, getWidth(), getHeight() / 2, Justification::centred);
    }
    else
    {
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (0.75f));
        g.drawText ("SHOW", 0, 3, getWidth(), getHeight() / 2, Justification::centred);
    }

    g.drawText ("AVG", 0, getHeight() / 2, getWidth(), getHeight() / 2 - 3, Justification::centred);
}

// BEGIN PUBLIC ACCESSOR DELEGATES

float CanvasOptionsBar::getRMSLowBound() const
{
    return rmsSubOptionComponent->getRMSLowBound();
}

float CanvasOptionsBar::getRMSHiBound() const
{
    return rmsSubOptionComponent->getRMSHiBound();
}

float CanvasOptionsBar::getRMSBoundSpread() const
{
    return rmsSubOptionComponent->getRMSBoundSpread();
}

float CanvasOptionsBar::getFFTLowBound() const
{
    return fftSubOptionComponent->getFFTLowBound();
}

float CanvasOptionsBar::getFFTHiBound() const
{
    return fftSubOptionComponent->getFFTHiBound();
}

float CanvasOptionsBar::getFFTBoundSpread() const
{
    return fftSubOptionComponent->getFFTBoundSpread();
}

int CanvasOptionsBar::getFFTCenterFrequencyBin() const
{
    return fftSubOptionComponent->getFFTSamplingBin();
}

void CanvasOptionsBar::setFFTParams (const int numBins, const float sampleRate)
{
    fftSubOptionComponent->setSampleRate (sampleRate);
    fftSubOptionComponent->setFFTSize (numBins);
}

float CanvasOptionsBar::getSpikeRateLowBound() const
{
    return spikeRateSubOptionComponent->getSpikeRateLowBound();
}

float CanvasOptionsBar::getSpikeRateHiBound() const
{
    return spikeRateSubOptionComponent->getSpikeRateHiBound();
}

float CanvasOptionsBar::getSpikeRateBoundSpread() const
{
    return spikeRateSubOptionComponent->getSpikeRateBoundSpread();
}

float CanvasOptionsBar::getSpikeRateThreshold() const
{
    return spikeRateSubOptionComponent->getSpikeRateThreshold();
}

void CanvasOptionsBar::saveParameters (XmlElement* xml)
{
    XmlElement* xmlNode = xml->createNewChildElement ("OPTIONS");

    xmlNode->setAttribute ("renderMode", renderModeSelection->getSelectedId());

    xmlNode->setAttribute ("rmsLow", getRMSLowBound());
    xmlNode->setAttribute ("rmsHi", getRMSHiBound());

    xmlNode->setAttribute ("fftLow", getFFTLowBound());
    xmlNode->setAttribute ("fftHi", getFFTHiBound());
    xmlNode->setAttribute ("fftFreqBin", fftSubOptionComponent->getFFTFrequency());

    xmlNode->setAttribute ("spikeLow", getSpikeRateLowBound());
    xmlNode->setAttribute ("spikeHi", getSpikeRateHiBound());
    xmlNode->setAttribute ("spikeThreshold", getSpikeRateThreshold());

    xmlNode->setAttribute ("colourScheme", colourSchemeSelection->getSelectedId());

    xmlNode->setAttribute ("showAverageView", showAverageViewButton->getToggleState());

    float rollingWindowSize, averageWindowPre, averageWindowPost;

    timeScale->getWindowSize (&rollingWindowSize, &averageWindowPre, &averageWindowPost);

    xmlNode->setAttribute ("rollingWindowSize", rollingWindowSize);
    xmlNode->setAttribute ("averageWindowPre", averageWindowPre);
    xmlNode->setAttribute ("averageWindowPost", averageWindowPost);
}

void CanvasOptionsBar::loadParameters (XmlElement* xml)
{
    XmlElement* xmlNode = xml->getChildByName ("OPTIONS");

    if (xmlNode)
    {
        renderModeSelection->setSelectedId (xmlNode->getIntAttribute ("renderMode", 1), sendNotification);

        rmsSubOptionComponent->setRMSBounds (xmlNode->getStringAttribute ("rmsLow", String()),
                                             xmlNode->getStringAttribute ("rmsHi", String()));

        fftSubOptionComponent->setFFTParams (xmlNode->getStringAttribute ("fftLow", String()),
                                             xmlNode->getStringAttribute ("fftHi", String()),
                                             xmlNode->getStringAttribute ("fftFreqBin", String()));

        spikeRateSubOptionComponent->setSpikeRateParams (xmlNode->getStringAttribute ("spikeLow", String()),
                                                         xmlNode->getStringAttribute ("spikeHi", String()),
                                                         xmlNode->getStringAttribute ("spikeThreshold", String()));

        colourSchemeSelection->setSelectedId (xmlNode->getIntAttribute ("colourScheme", 1));

        showAverageViewButton->setToggleState (xmlNode->getBoolAttribute ("showAverageView", false), sendNotification);

        float rollingWindowSize, averageWindowPre, averageWindowPost;

        rollingWindowSize = xmlNode->getDoubleAttribute ("rollingWindowSize", 8.0f);
        averageWindowPre = xmlNode->getDoubleAttribute ("averageWindowPre", 0.5f);
        averageWindowPost = xmlNode->getDoubleAttribute ("averageWindowPost", 0.5f);

        timeScale->setRollingViewWindowSize (rollingWindowSize);
        timeScale->setAverageViewWindowSize (averageWindowPre, averageWindowPost);
    }
}

bool CanvasOptionsBar::isScrollBarVisible(int newWidth)
{
    return newWidth < minWidth;
}

#pragma mark - RMSSubOptionComponent -

RMSSubOptionComponent::RMSSubOptionComponent (Font labelFont)
    : labelFont (labelFont), lowValueBound (0), hiValueBound (250)
{
    // low value plotting threshold
    lowValueBoundLabel.reset (new Label ("lowValueBoundLabel", "Low:"));
    lowValueBoundLabel->setFont (labelFont);
    addAndMakeVisible (lowValueBoundLabel.get());

    lowValueBoundSelectionOptions.addArray ({ "0",
                                              "50",
                                              "100",
                                              "150",
                                              "200",
                                              "250",
                                              "300",
                                              "350",
                                              "400",
                                              "450",
                                              "500" });
    lowValueBoundSelection.reset (new ComboBox ("lowValueBoundSelection"));
    lowValueBoundSelection->addItemList (lowValueBoundSelectionOptions, 1);
    lowValueBoundSelection->setEditableText (true);
    lowValueBoundSelection->addListener (this);
    lowValueBoundSelection->setSelectedId (1, dontSendNotification);
    addAndMakeVisible (lowValueBoundSelection.get());

    // hi value plotting threshold
    hiValueBoundLabel.reset (new Label ("hiValueBoundLabel", "High:"));
    hiValueBoundLabel->setFont (labelFont);
    addAndMakeVisible (hiValueBoundLabel.get());

    hiValueBoundSelectionOptions.addArray ({ "0",
                                             "50",
                                             "100",
                                             "150",
                                             "200",
                                             "250",
                                             "300",
                                             "350",
                                             "400",
                                             "450",
                                             "500" });
    hiValueBoundSelection.reset (new ComboBox ("hiValueBoundSelection"));
    hiValueBoundSelection->addItemList (hiValueBoundSelectionOptions, 1);
    hiValueBoundSelection->setEditableText (true);
    hiValueBoundSelection->addListener (this);
    hiValueBoundSelection->setSelectedId (6, dontSendNotification);
    addAndMakeVisible (hiValueBoundSelection.get());
}

RMSSubOptionComponent::~RMSSubOptionComponent()
{
}

void RMSSubOptionComponent::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::defaultText));
    g.drawRect (0, 0, getWidth(), getHeight());
    g.drawFittedText ("RMS OPTIONS", 0, 0, getWidth() - 5, getHeight(), Justification::centredRight, 1);
}

void RMSSubOptionComponent::resized()
{
    lowValueBoundLabel->setBounds (0, 0, 40, getHeight());
    lowValueBoundSelection->setBounds (lowValueBoundLabel->getRight(), 4, 60, getHeight() - 8);
    hiValueBoundLabel->setBounds (lowValueBoundSelection->getRight() + 10, 0, 50, getHeight());
    hiValueBoundSelection->setBounds (hiValueBoundLabel->getRight(), 4, 60, getHeight() - 8);
}

void RMSSubOptionComponent::addListener (ComboBox::Listener* listener)
{
    lowValueBoundSelection->addListener (listener);
    hiValueBoundSelection->addListener (listener);
}

void RMSSubOptionComponent::comboBoxChanged (ComboBox* cb)
{
    if (cb == lowValueBoundSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            // clip value if necessary
            if (val > 1000)
                val = 1000;

            lowValueBound = val;

            cb->setText (String (val));
        }
        else
        {
            lowValueBound = lowValueBoundSelectionOptions[cb->getSelectedItemIndex()].getFloatValue();
        }

        return;
    }

    if (cb == hiValueBoundSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            // clip value if necessary
            if (val > 1000)
                val = 1000;

            hiValueBound = val;

            cb->setText (String (val));
        }
        else
        {
            hiValueBound = lowValueBoundSelectionOptions[cb->getSelectedItemIndex()].getFloatValue();
        }

        return;
    }
}

float RMSSubOptionComponent::getRMSLowBound() const
{
    return lowValueBound;
}

float RMSSubOptionComponent::getRMSHiBound() const
{
    return hiValueBound;
}

float RMSSubOptionComponent::getRMSBoundSpread() const
{
    return hiValueBound - lowValueBound;
}

void RMSSubOptionComponent::setRMSBounds (String low, String high)
{
    if (low.isNotEmpty())
        lowValueBoundSelection->setText (low, sendNotification);

    if (high.isNotEmpty())
        hiValueBoundSelection->setText (high, sendNotification);
}

#pragma mark - FFTSubOptionComponent -

FFTSubOptionComponent::FFTSubOptionComponent (Font labelFont)
    : labelFont (labelFont), binSelectionValue (0), maxFreq (44100.0f / 2.0f)
{
    // low value plotting threshold
    lowValueBoundLabel.reset (new Label ("lowValueBoundLabel", "Low (dB):"));
    lowValueBoundLabel->setFont (labelFont);
    addAndMakeVisible (lowValueBoundLabel.get());

    lowValueBound = -100;
    lowValueBoundSelectionOptions.addArray ({ "-100", "-90", "-80", "-70", "-60", "-70", "-50", "-40", "-30" });
    lowValueBoundSelection.reset (new ComboBox ("lowValueBoundSelection"));
    lowValueBoundSelection->addItemList (lowValueBoundSelectionOptions, 1);
    lowValueBoundSelection->setEditableText (true);
    lowValueBoundSelection->addListener (this);
    lowValueBoundSelection->setSelectedId (1, dontSendNotification);
    addAndMakeVisible (lowValueBoundSelection.get());

    // hi value plotting threshold
    hiValueBoundLabel.reset (new Label ("hiValueBoundLabel", "High (dB):"));
    hiValueBoundLabel->setFont (labelFont);
    addAndMakeVisible (hiValueBoundLabel.get());

    hiValueBound = 0;
    hiValueBoundSelectionOptions.addArray ({ "0", "-5", "-10", "-15", "-20" });
    hiValueBoundSelection.reset (new ComboBox ("hiValueBoundSelection"));
    hiValueBoundSelection->addItemList (hiValueBoundSelectionOptions, 1);
    hiValueBoundSelection->setEditableText (true);
    hiValueBoundSelection->addListener (this);
    hiValueBoundSelection->setSelectedId (1, dontSendNotification);
    addAndMakeVisible (hiValueBoundSelection.get());

    // bin selection
    binSelectionLabel.reset (new Label ("binSelectionLabel", "Center Freq.:"));
    binSelectionLabel->setFont (labelFont);
    addAndMakeVisible (binSelectionLabel.get());

    binSelectionOptions.addArray ({ "1", "8", "16", "32", "64", "128", "256" });
    binSelection.reset (new ComboBox ("binSelection"));
    binSelection->addItemList (binSelectionOptions, 1);
    binSelection->setEditableText (false);
    binSelection->addListener (this);
    binSelection->setSelectedId (binSelectionValue + 1, dontSendNotification);
    addAndMakeVisible (binSelection.get());
}

FFTSubOptionComponent::~FFTSubOptionComponent()
{
}

void FFTSubOptionComponent::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::defaultText));
    g.drawRect (0, 0, getWidth(), getHeight());
    g.drawFittedText ("FFT OPTIONS", 0, 0, getWidth() - 5, getHeight(), Justification::centredRight, 1);
}

void FFTSubOptionComponent::resized()
{
    lowValueBoundLabel->setBounds (0, 0, 70, getHeight());
    lowValueBoundSelection->setBounds (lowValueBoundLabel->getRight(), 4, 60, getHeight() - 8);

    hiValueBoundLabel->setBounds (lowValueBoundSelection->getRight() + 10, 0, 75, getHeight());
    hiValueBoundSelection->setBounds (hiValueBoundLabel->getRight(), 4, 60, getHeight() - 8);

    binSelectionLabel->setBounds (hiValueBoundSelection->getRight() + 10, 0, 100, getHeight());
    binSelection->setBounds (binSelectionLabel->getRight(), 4, 80, getHeight() - 8);
}

namespace
{
// helper function for center frequency bin selection, 0 and numBins inclusive
inline int freqToBinIndex (float freq, float maxFreq, int numBins)
{
    return std::round ((freq / maxFreq) * numBins);
}
} // namespace

void FFTSubOptionComponent::addListener (ComboBox::Listener* listener)
{
    lowValueBoundSelection->addListener (listener);
    hiValueBoundSelection->addListener (listener);
    binSelection->addListener (listener);
}

void FFTSubOptionComponent::comboBoxChanged (ComboBox* cb)
{
    if (cb == lowValueBoundSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            if (val > 100)
                val = 100;

            val *= -1;

            lowValueBound = val;

            cb->setText (String (val));
        }
        else
        {
            lowValueBound = cb->getText().getFloatValue();
        }
        return;
    }
    if (cb == hiValueBoundSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            if (val > 100)
                val = 100;

            val *= -1;

            hiValueBound = val;

            cb->setText (String (val));
        }
        else
        {
            hiValueBound = cb->getText().getFloatValue();
        }
        return;
    }
    if (cb == binSelection.get())
    {
        binSelectionValue = freqToBinIndex (cb->getText().getFloatValue(), maxFreq, fftSize / 2);
        return;
    }
}

void FFTSubOptionComponent::setSampleRate (const float sampleRate)
{
    this->sampleRate = sampleRate;
    maxFreq = sampleRate / 2.0f;
}

void FFTSubOptionComponent::setFFTSize (const int numBins_)
{
    fftSize = numBins_;
    this->numBins = numBins_ / 2 + 1;
}

float FFTSubOptionComponent::getFFTLowBound() const
{
    return lowValueBound;
}

float FFTSubOptionComponent::getFFTHiBound() const
{
    return hiValueBound;
}

float FFTSubOptionComponent::getFFTBoundSpread() const
{
    return fabsf (hiValueBound - lowValueBound);
}

int FFTSubOptionComponent::getFFTSamplingBin() const
{
    return binSelectionValue;
}

int FFTSubOptionComponent::getFFTFrequency() const
{
    return binSelection->getText().getIntValue();
}

void FFTSubOptionComponent::setFFTParams (String low, String high, String bin)
{
    if (low.isNotEmpty())
        lowValueBoundSelection->setText (low, sendNotification);

    if (high.isNotEmpty())
        hiValueBoundSelection->setText (high, sendNotification);

    if (bin.isNotEmpty())
        binSelection->setText (bin, sendNotification);
}

#pragma mark - SpikeRateSubOptionComponent -

SpikeRateSubOptionComponent::SpikeRateSubOptionComponent (Font labelFont)
    : labelFont (labelFont)
{
    // low bound plotting threshold
    lowValueBoundLabel.reset (new Label ("lowValueBoundLabel", "Low:"));
    lowValueBoundLabel->setFont (labelFont);
    addAndMakeVisible (lowValueBoundLabel.get());

    lowValueBoundSelectionOptions.add ("0");
    lowValueBoundSelection.reset (new ComboBox ("lowValueBoundSelection"));
    lowValueBoundSelection->addItemList (lowValueBoundSelectionOptions, 1);
    lowValueBoundSelection->setEditableText (true);
    lowValueBoundSelection->addListener (this);
    lowValueBoundSelection->setSelectedId (1, dontSendNotification);
    lowValueBound = 0;
    addAndMakeVisible (lowValueBoundSelection.get());

    // hi bound plotting threshold
    hiValueBoundLabel.reset (new Label ("hiValueBoundLabel", "High:"));
    hiValueBoundLabel->setFont (labelFont);
    addAndMakeVisible (hiValueBoundLabel.get());

    hiValueBoundSelectionOptions.add ("10000");
    hiValueBoundSelection.reset (new ComboBox ("hiValueBoundSelection"));
    hiValueBoundSelection->addItemList (hiValueBoundSelectionOptions, 1);
    hiValueBoundSelection->setEditableText (true);
    hiValueBoundSelection->addListener (this);
    hiValueBoundSelection->setSelectedId (1, dontSendNotification);
    hiValueBound = 10000;
    addAndMakeVisible (hiValueBoundSelection.get());

    // spike onset threshold
    thresholdSelectionLabel.reset (new Label ("thresholdSelectionLabel", "Threshold (uV):"));
    thresholdSelectionLabel->setFont (labelFont);
    addAndMakeVisible (thresholdSelectionLabel.get());

    thresholdSelectionOptions.addArray ({ "-25", "-50", "-75", "-100", "-150", "-200" });
    thresholdSelection.reset (new ComboBox ("thresholdSelection"));
    thresholdSelection->addItemList (thresholdSelectionOptions, 1);
    thresholdSelection->setEditableText (true);
    thresholdSelection->setSelectedId (2, dontSendNotification);
    thresholdSelection->addListener (this);
    threshold = -50;
    addAndMakeVisible (thresholdSelection.get());
}

SpikeRateSubOptionComponent::~SpikeRateSubOptionComponent()
{
}

void SpikeRateSubOptionComponent::paint (Graphics& g)
{
    g.setColour (findColour (ThemeColours::defaultText));
    g.drawRect (0, 0, getWidth(), getHeight());
    g.drawFittedText ("SPIKE RATE OPTIONS", 0, 0, getWidth() - 5, getHeight(), Justification::centredRight, 1);
}

void SpikeRateSubOptionComponent::resized()
{
    lowValueBoundLabel->setBounds (0, 0, 40, getHeight());
    lowValueBoundSelection->setBounds (lowValueBoundLabel->getRight(), 4, 60, getHeight() - 8);

    hiValueBoundLabel->setBounds (lowValueBoundSelection->getRight() + 10, 0, 50, getHeight());
    hiValueBoundSelection->setBounds (hiValueBoundLabel->getRight(), 4, 70, getHeight() - 8);

    thresholdSelectionLabel->setBounds (hiValueBoundSelection->getRight() + 10, 0, 120, getHeight());
    thresholdSelection->setBounds (thresholdSelectionLabel->getRight(), 4, 60, getHeight() - 8);
}

void SpikeRateSubOptionComponent::addListener (ComboBox::Listener* listener)
{
    lowValueBoundSelection->addListener (listener);
    hiValueBoundSelection->addListener (listener);
    thresholdSelection->addListener (listener);
}

void SpikeRateSubOptionComponent::comboBoxChanged (ComboBox* cb)
{
    if (cb == lowValueBoundSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            if (val > 5000)
                val = 5000;

            lowValueBound = val;

            cb->setText (String (val));
        }
        else
        {
            auto val = cb->getText().getFloatValue();

            lowValueBound = val;
        }
        return;
    }

    if (cb == hiValueBoundSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            if (val > 10000)
                val = 10000;

            hiValueBound = val;

            cb->setText (String (val));
        }
        else
        {
            auto val = cb->getText().getFloatValue();

            hiValueBound = val;
        }
        return;
    }

    if (cb == thresholdSelection.get())
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf (cb->getText().getFloatValue());

            if (val < 10)
                val = 10;
            else if (val > 500)
                val = 500;

            val *= -1;

            threshold = val;

            cb->setText (String (val));
        }
        else // otherwise get the preset value
        {
            auto val = cb->getText().getFloatValue();

            threshold = val;
        }

        return;
    }
}

float SpikeRateSubOptionComponent::getSpikeRateLowBound() const
{
    return lowValueBound;
}

float SpikeRateSubOptionComponent::getSpikeRateHiBound() const
{
    return hiValueBound;
}

float SpikeRateSubOptionComponent::getSpikeRateBoundSpread() const
{
    return fabs (hiValueBound - lowValueBound);
}

float SpikeRateSubOptionComponent::getSpikeRateThreshold() const
{
    return threshold;
}

void SpikeRateSubOptionComponent::setSpikeRateParams (String low, String high, String threshold)
{
    if (low.isNotEmpty())
        lowValueBoundSelection->setText (low, sendNotification);

    if (high.isNotEmpty())
        hiValueBoundSelection->setText (high, sendNotification);

    if (threshold.isNotEmpty())
        thresholdSelection->setText (threshold, sendNotification);
}
