//
//  CanvasOptionsBar.cpp
//  ProbeViewerPlugin
//
//  Created by Kelly Fox on 10/19/17.
//  Copyright © 2017 Allen Institute. All rights reserved.
//

#include "CanvasOptionsBar.hpp"

#include "ChannelViewCanvas.hpp"
#include "../ProbeViewerCanvas.h"
#include "../Utilities/ColourScheme.hpp"

using namespace ProbeViewer;

CanvasOptionsBar::CanvasOptionsBar(class ChannelViewCanvas* channelsView)
: channelsView(channelsView)
, marginWidth(0)
, labelFont("Fira Code", "Regular", 16.0f)
, labelColour(100, 100, 100)
{
    rmsSubOptionComponent = new RMSSubOptionComponent(labelFont, labelColour);
    fftSubOptionComponent = new FFTSubOptionComponent(labelFont, labelColour);
    spikeRateSubOptionComponent = new SpikeRateSubOptionComponent(labelFont, labelColour);
    
    currentSubOptionComponent = rmsSubOptionComponent;
    addAndMakeVisible(currentSubOptionComponent);
    
    renderModeSelectionLabel = new Label("renderModeSelectionLabel", "Render Mode");
    renderModeSelectionLabel->setFont(labelFont);
    renderModeSelectionLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(renderModeSelectionLabel);
    
    StringArray renderModeNames = {"RMS Signal", "Freq. Band Power", "Spike Rate"};
    renderModeSelection = new ComboBox("renderModeSelection");
    renderModeSelection->addItemList(renderModeNames, 1);
    renderModeSelection->setEditableText(false);
    renderModeSelection->addListener(this);
    renderModeSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(renderModeSelection);
    
    
    
    // colour scheme options
    colourSchemeSelectionLabel = new Label("colourSchemeSelectionLabel", "Colour\nScheme");
    colourSchemeSelectionLabel->setFont(labelFont);
    colourSchemeSelectionLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(colourSchemeSelectionLabel);
    
    StringArray colourSchemeNames = {"Inferno", "Plasma", "Magma", "Viridis", "Jet"};
    colourSchemeSelection = new ComboBox("colourSchemeSelection");
    colourSchemeSelection->addItemList(colourSchemeNames, 1);
    colourSchemeSelection->setEditableText(false);
    colourSchemeSelection->addListener(this);
    colourSchemeSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(colourSchemeSelection);
    
    setBufferedToImage(true);
}

CanvasOptionsBar::~CanvasOptionsBar()
{ }

void CanvasOptionsBar::paint(Graphics& g)
{
    g.setColour(Colour(25, 25, 25));
    g.fillRect(0, 0, getWidth(), getHeight());
    
    g.setColour(Colour(80, 80, 80));
    g.drawLine(marginWidth, 0, marginWidth, getHeight(), 1);
}

void CanvasOptionsBar::resized()
{
    renderModeSelectionLabel->setBounds(0, 0, 95, getHeight());
    renderModeSelection->setBounds(renderModeSelectionLabel->getRight(), 2, 100, getHeight() - 4);
    
    int colourSchemeOffset = 700;
    if (getWidth() > colourSchemeOffset) colourSchemeOffset = getWidth();
    colourSchemeSelectionLabel->setBounds(colourSchemeOffset - 170, 0, 70, getHeight());
    colourSchemeSelection->setBounds(colourSchemeSelectionLabel->getRight(), 2, 90, getHeight() - 4);
    
    Rectangle<int> subOptionBounds(marginWidth + 3, 0, colourSchemeOffset - marginWidth - 170 - 3, getHeight());
    rmsSubOptionComponent->setBounds(subOptionBounds);
    fftSubOptionComponent->setBounds(subOptionBounds);
    spikeRateSubOptionComponent->setBounds(subOptionBounds);
}

void CanvasOptionsBar::comboBoxChanged(ComboBox *cb)
{
    if (cb == renderModeSelection)
    {
        removeChildComponent(currentSubOptionComponent);
        
        RenderMode renderMode;
        
        switch(cb->getSelectedId())
        {
            case 1:
                renderMode = RenderMode::RMS;
                currentSubOptionComponent = rmsSubOptionComponent;
                break;
                
            case 2:
                renderMode = RenderMode::FFT;
                currentSubOptionComponent = fftSubOptionComponent;
                break;
                
            case 3:
            default:
                renderMode = RenderMode::SPIKE_RATE;
                currentSubOptionComponent = spikeRateSubOptionComponent;
                break;
        }
        
        addAndMakeVisible(currentSubOptionComponent);
        
        channelsView->setCurrentRenderMode(renderMode);
    }
    else if (cb == colourSchemeSelection)
    {
        ColourSchemeId colourSchemeId;
        
        switch(cb->getSelectedId())
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
        
        channelsView->setCurrentColourScheme(colourSchemeId);
    }
}

void CanvasOptionsBar::setMarginOffset(float marginOffset)
{
    marginWidth = marginOffset;
    resized();
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

void CanvasOptionsBar::setFFTParams(const int numBins, const float sampleRate)
{
    fftSubOptionComponent->setSampleRate(sampleRate);
    fftSubOptionComponent->setFFTSize(numBins);
    
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

void CanvasOptionsBar::saveParameters(XmlElement* xml)
{
    // XmlElement* xmlNode = xml->createNewChildElement("CANVAS OPTIONS");

    // xmlNode->setAttribute("renderMode", renderModeSelection->getSelectedId());

    // xmlNode->setAttribute("renderMode", renderModeSelection->getSelectedId());

}

void CanvasOptionsBar::loadParameters(XmlElement* xml)
{
    
}

#pragma mark - RMSSubOptionComponent -

RMSSubOptionComponent::RMSSubOptionComponent(Font labelFont, Colour labelColour)
: labelFont(labelFont)
, labelColour(labelColour)
, lowValueBound(0)
, hiValueBound(250)
{
    // low value plotting threshold
    lowValueBoundLabel = new Label("lowValueBoundLabel", "Low:");
    lowValueBoundLabel->setFont(labelFont);
    lowValueBoundLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(lowValueBoundLabel);
    
    lowValueBoundSelectionOptions.addArray({
        "0",
        "50",
        "100",
        "150",
        "200",
        "250",
        "300",
        "350",
        "400",
        "450",
        "500"
    });
    lowValueBoundSelection = new ComboBox("lowValueBoundSelection");
    lowValueBoundSelection->addItemList(lowValueBoundSelectionOptions, 1);
    lowValueBoundSelection->setEditableText(true);
    lowValueBoundSelection->addListener(this);
    lowValueBoundSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(lowValueBoundSelection);
    
    // hi value plotting threshold
    hiValueBoundLabel = new Label("hiValueBoundLabel", "High:");
    hiValueBoundLabel->setFont(labelFont);
    hiValueBoundLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(hiValueBoundLabel);
    
    hiValueBoundSelectionOptions.addArray({
        "0",
        "50",
        "100",
        "150",
        "200",
        "250",
        "300",
        "350",
        "400",
        "450",
        "500"
    });
    hiValueBoundSelection = new ComboBox("hiValueBoundSelection");
    hiValueBoundSelection->addItemList(hiValueBoundSelectionOptions, 1);
    hiValueBoundSelection->setEditableText(true);
    hiValueBoundSelection->addListener(this);
    hiValueBoundSelection->setSelectedId(6, dontSendNotification);
    addAndMakeVisible(hiValueBoundSelection);
}

RMSSubOptionComponent::~RMSSubOptionComponent()
{ }

void RMSSubOptionComponent::paint(Graphics& g)
{
    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, getWidth(), getHeight());
    g.drawFittedText("RMS SUB OPTIONS", 0, 0, getWidth() - 5, getHeight(), Justification::centredRight, 1);
}

void RMSSubOptionComponent::resized()
{
    lowValueBoundLabel->setBounds(0, 0, 40, getHeight());
    lowValueBoundSelection->setBounds(lowValueBoundLabel->getRight(), 2, 60, getHeight() - 4);
    hiValueBoundLabel->setBounds(lowValueBoundSelection->getRight() + 10, 0, 50, getHeight());
    hiValueBoundSelection->setBounds(hiValueBoundLabel->getRight(), 2, 60, getHeight() - 4);
}

void RMSSubOptionComponent::comboBoxChanged(ComboBox* cb)
{
    if (cb == lowValueBoundSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            std::cout << val << std::endl;
            
            // clip value if necessary
            if (val > 1000) val = 1000;
            
            lowValueBound = val;
            
            cb->setText(String(val));
        }
        else
        {
            lowValueBound = lowValueBoundSelectionOptions[cb->getSelectedItemIndex()].getFloatValue();
        }
        
        return;
    }
    
    if (cb == hiValueBoundSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            // clip value if necessary
            if (val > 1000) val = 1000;
            
            hiValueBound = val;
            
            cb->setText(String(val));
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




#pragma mark - FFTSubOptionComponent -

FFTSubOptionComponent::FFTSubOptionComponent(Font labelFont, Colour labelColour)
: labelFont(labelFont)
, labelColour(labelColour)
, binSelectionValue(0)
, maxFreq(44100.0f / 2.0f)
{
    // low value plotting threshold
    lowValueBoundLabel = new Label("lowValueBoundLabel", "Low (dB):");
    lowValueBoundLabel->setFont(labelFont);
    lowValueBoundLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(lowValueBoundLabel);
    
    lowValueBound = -100;
    lowValueBoundSelectionOptions.addArray({
        "-100", "-90", "-80", "-70", "-60", "-70", "-50", "-40", "-30"
    });
    lowValueBoundSelection = new ComboBox("lowValueBoundSelection");
    lowValueBoundSelection->addItemList(lowValueBoundSelectionOptions, 1);
    lowValueBoundSelection->setEditableText(true);
    lowValueBoundSelection->addListener(this);
    lowValueBoundSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(lowValueBoundSelection);
    
    
    
    // hi value plotting threshold
    hiValueBoundLabel = new Label("hiValueBoundLabel", "High (dB):");
    hiValueBoundLabel->setFont(labelFont);
    hiValueBoundLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(hiValueBoundLabel);
    
    hiValueBound = 0;
    hiValueBoundSelectionOptions.addArray({
        "0", "-5", "-10", "-15", "-20"
    });
    hiValueBoundSelection = new ComboBox("hiValueBoundSelection");
    hiValueBoundSelection->addItemList(hiValueBoundSelectionOptions, 1);
    hiValueBoundSelection->setEditableText(true);
    hiValueBoundSelection->addListener(this);
    hiValueBoundSelection->setSelectedId(1, dontSendNotification);
    addAndMakeVisible(hiValueBoundSelection);
    
    
    
    // bin selection
    binSelectionLabel = new Label("binSelectionLabel", "Center Frequency:");
    binSelectionLabel->setFont(labelFont);
    binSelectionLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(binSelectionLabel);
    
    binSelectionOptions.addArray({"1", "8", "16", "32", "64", "128", "256"});
    binSelection = new ComboBox("binSelection");
    binSelection->addItemList(binSelectionOptions, 1);
    binSelection->setEditableText(false);
    binSelection->addListener(this);
    binSelection->setSelectedId(binSelectionValue + 1, dontSendNotification);
    addAndMakeVisible(binSelection);
    
}

FFTSubOptionComponent::~FFTSubOptionComponent()
{ }

void FFTSubOptionComponent::paint(Graphics& g)
{
    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, getWidth(), getHeight());
    g.drawFittedText("FFT SUB OPTIONS", 0, 0, getWidth() - 5, getHeight(), Justification::centredRight, 1);
}

void FFTSubOptionComponent::resized()
{
    lowValueBoundLabel->setBounds(0, 0, 70, getHeight());
    lowValueBoundSelection->setBounds(lowValueBoundLabel->getRight(), 2, 60, getHeight() - 4);

    hiValueBoundLabel->setBounds(lowValueBoundSelection->getRight() + 10, 0, 75, getHeight());
    hiValueBoundSelection->setBounds(hiValueBoundLabel->getRight(), 2, 60, getHeight() - 4);

    binSelectionLabel->setBounds(hiValueBoundSelection->getRight() + 10, 0, 130, getHeight());
    binSelection->setBounds(binSelectionLabel->getRight(), 2, 80, getHeight() - 4);
}

namespace {
    // helper function for center frequency bin selection, 0 and numBins inclusive
    inline int freqToBinIndex(float freq, float maxFreq, int numBins)
    {
        return std::round((freq/maxFreq) * numBins);
    }
}

void FFTSubOptionComponent::comboBoxChanged(ComboBox* cb)
{
    if (cb == lowValueBoundSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val > 100) val = 100;
            
            val *= -1;
            
            lowValueBound = val;
            
            cb->setText(String(val));
        }
        else
        {
            lowValueBound = cb->getText().getFloatValue();
        }
        return;
    }
    if (cb == hiValueBoundSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val > 100) val = 100;
            
            val *= -1;
            
            hiValueBound = val;
            
            cb->setText(String(val));
        }
        else
        {
            hiValueBound = cb->getText().getFloatValue();
        }
        return;
    }
    if (cb == binSelection)
    {
        binSelectionValue = freqToBinIndex(cb->getText().getFloatValue(), maxFreq, fftSize/2);
        return;
    }
}

void FFTSubOptionComponent::setSampleRate(const float sampleRate)
{
    this->sampleRate = sampleRate;
    maxFreq = sampleRate / 2.0f;
}

void FFTSubOptionComponent::setFFTSize(const int numBins_)
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
    return fabsf(hiValueBound - lowValueBound);
}

int FFTSubOptionComponent::getFFTSamplingBin() const
{
    return binSelectionValue;
}





#pragma mark - SpikeRateSubOptionComponent -

SpikeRateSubOptionComponent::SpikeRateSubOptionComponent(Font labelFont, Colour labelColour)
: labelFont(labelFont)
, labelColour(labelColour)
{
    // low bound plotting threshold
    lowValueBoundLabel = new Label("lowValueBoundLabel", "Low:");
    lowValueBoundLabel->setFont(labelFont);
    lowValueBoundLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(lowValueBoundLabel);
    
    lowValueBoundSelectionOptions.add("0");
    lowValueBoundSelection = new ComboBox("lowValueBoundSelection");
    lowValueBoundSelection->addItemList(lowValueBoundSelectionOptions, 1);
    lowValueBoundSelection->setEditableText(true);
    lowValueBoundSelection->addListener(this);
    lowValueBoundSelection->setSelectedId(1, dontSendNotification);
    lowValueBound = 0;
    addAndMakeVisible(lowValueBoundSelection);
    
    
    // hi bound plotting threshold
    hiValueBoundLabel = new Label("hiValueBoundLabel", "High:");
    hiValueBoundLabel->setFont(labelFont);
    hiValueBoundLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(hiValueBoundLabel);
    
    hiValueBoundSelectionOptions.add("10000");
    hiValueBoundSelection = new ComboBox("hiValueBoundSelection");
    hiValueBoundSelection->addItemList(hiValueBoundSelectionOptions, 1);
    hiValueBoundSelection->setEditableText(true);
    hiValueBoundSelection->addListener(this);
    hiValueBoundSelection->setSelectedId(1, dontSendNotification);
    hiValueBound = 10000;
    addAndMakeVisible(hiValueBoundSelection);
    
    
    // spike onset threshold
    thresholdSelectionLabel = new Label("thresholdSelectionLabel", "Spike Onset Threshold:");
    thresholdSelectionLabel->setFont(labelFont);
    thresholdSelectionLabel->setColour(Label::textColourId, labelColour);
    addAndMakeVisible(thresholdSelectionLabel);
    
    thresholdSelectionOptions.addArray({
        "-25", "-50",
        "-75", "-100",
        "-150", "-200"
    });
    thresholdSelection = new ComboBox("thresholdSelection");
    thresholdSelection->addItemList(thresholdSelectionOptions, 1);
    thresholdSelection->setEditableText(true);
    thresholdSelection->setSelectedId(2, dontSendNotification);
    thresholdSelection->addListener(this);
    threshold = -50;
    addAndMakeVisible(thresholdSelection);
}

SpikeRateSubOptionComponent::~SpikeRateSubOptionComponent()
{ }

void SpikeRateSubOptionComponent::paint(Graphics& g)
{
    g.setColour(Colours::darkgrey);
    g.drawRect(0, 0, getWidth(), getHeight());
    g.drawFittedText("SPIKE RATE SUB OPTIONS", 0, 0, getWidth() - 5, getHeight(), Justification::centredRight, 1);
}

void SpikeRateSubOptionComponent::resized()
{
    lowValueBoundLabel->setBounds(0, 0, 40, getHeight());
    lowValueBoundSelection->setBounds(lowValueBoundLabel->getRight(), 2, 60, getHeight() - 4);

    hiValueBoundLabel->setBounds(lowValueBoundSelection->getRight() + 10, 0, 50, getHeight());
    hiValueBoundSelection->setBounds(hiValueBoundLabel->getRight(), 2, 70, getHeight() - 4);
    
    thresholdSelectionLabel->setBounds(hiValueBoundSelection->getRight() + 10, 0, 150, getHeight());
    thresholdSelection->setBounds(thresholdSelectionLabel->getRight(), 2, 60, getHeight() - 4);
}

void SpikeRateSubOptionComponent::comboBoxChanged(ComboBox* cb)
{
    if (cb == lowValueBoundSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val > 5000) val = 5000;
            
            lowValueBound = val;
            
            cb->setText(String(val));
        }
        else
        {
            auto val = cb->getText().getFloatValue();
            
            lowValueBound = val;
        }
        return;
    }
    
    if (cb == hiValueBoundSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val > 10000) val = 10000;
            
            hiValueBound = val;
            
            cb->setText(String(val));
        }
        else
        {
            auto val = cb->getText().getFloatValue();
            
            hiValueBound = val;
        }
        return;
    }
    
    if (cb == thresholdSelection)
    {
        // if custom value
        if (cb->getSelectedId() == 0)
        {
            auto val = fabsf(cb->getText().getFloatValue());
            
            if (val < 10) val = 10;
            else if (val > 500) val = 500;
            
            val *= -1;
            
            threshold = val;
            
            cb->setText(String(val));
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
    return fabs(hiValueBound - lowValueBound);
}

float SpikeRateSubOptionComponent::getSpikeRateThreshold() const
{
    return threshold;
}
