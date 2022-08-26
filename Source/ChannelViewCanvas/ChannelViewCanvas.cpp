/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2017 Open Ephys
 
 ------------------------------------------------------------------
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#include "ChannelViewCanvas.hpp"
#include "../ProbeViewerCanvas.h"

#include "../NeuropixInterface/NeuropixInterface.hpp"
#include "../Utilities/ColourScheme.hpp"
#include "CanvasOptionsBar.hpp"

using namespace ProbeViewer;

#pragma mark - ChannelViewCanvas -

ChannelViewCanvas::ChannelViewCanvas(ProbeViewerCanvas* canvas)
: isDirty(false)
, numPixelUpdates(0)
, canvas(canvas)
, channelHeight(10)
, colourSchemeId(ColourSchemeId::INFERNO)
, screenBufferImage(Image::RGB, CHANNEL_DISPLAY_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * 394, false)
, renderMode(RenderMode::RMS)
, frontBackBufferPixelOffset(0)
, frontBufferIndex(0)
, fullRedraw(false)
, numChannels(0)
{
    
}

ChannelViewCanvas::~ChannelViewCanvas()
{ }

void ChannelViewCanvas::paint(Graphics& g)
{
    renderTilesToScreenBufferImage();
    
    const float verticalScale = (getChannelHeight() * numChannels) / getFrontBufferPtr()->height;
    const float horizontalScale = getWidth() / float(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH);
    
    const auto transform = AffineTransform::scale(horizontalScale, verticalScale);
    
    g.drawImageTransformed(screenBufferImage, transform);
}

void ChannelViewCanvas::resized()
{
    fullRedraw = true;
    repaint();
}

void ChannelViewCanvas::refresh()
{
    if (isDirty.get()) // isDirty is set true when a new pixel has been pushed, and false when all queued pix are drawn
    {
        while (numPixelUpdates > 0)
        {
            // get the front buffer and create a BitmapData accessor object for it
            auto frontBufferPtr = getFrontBufferPtr();
            auto frontTile = frontBufferPtr->getTileForRenderMode(RenderMode::RMS);
            
            Image::BitmapData bdFrontBufferBitmap(*frontTile, 0, 0, frontBufferPtr->width, frontBufferPtr->height, Image::BitmapData::ReadWriteMode::writeOnly);
            
            // paint the pixel updates for each channel to the bitmap data
            for (auto channel : channels)
            {
                channel->pxPaint();
            }
            --numPixelUpdates;
            tick();
        }
        repaint(0, 0, getWidth(), getHeight());
        isDirty.set(false);
    }
}

void ChannelViewCanvas::updateViewSettings()
{
    numChannels = canvas->getNumChannels();

    displayBitmapTiles.clear();

    if(numChannels > 0)
    {

        screenBufferImage = Image(Image::RGB, CHANNEL_DISPLAY_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * numChannels, false);

        // get number of tiles that covers display width, the drawing mechanics will need
        // to be adapted for display widths other than the current fixed value of 1920 pixels wide
        int numTiles = ceil(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH / float(ChannelViewCanvas::CHANNEL_DISPLAY_TILE_WIDTH));
        
        for (int i = 0; i < numTiles; ++i)
        {
            auto tile = new BitmapRenderTile(CHANNEL_DISPLAY_TILE_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * numChannels, numChannels);
            displayBitmapTiles.add(tile);
        }
    }
}

void ChannelViewCanvas::renderTilesToScreenBufferImage()
{
    Graphics gScreenBuffer(screenBufferImage);
    
    if (fullRedraw)
    {
        for (int backBufferIdx = 0; backBufferIdx < displayBitmapTiles.size(); ++backBufferIdx)
        {
            // draw each tile
            gScreenBuffer.drawImageAt(*displayBitmapTiles[backBufferIdx]->getTileForRenderMode(renderMode), backBufferIdx * CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        }
        
        fullRedraw = false;
    }
    else
    {
        const bool shouldRedrawLastBufferToo = true;
        
        gScreenBuffer.drawImageAt(*getFrontBufferPtr()->getTileForRenderMode(renderMode), frontBufferIndex * CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        
        if (shouldRedrawLastBufferToo)
        {
            const int prevIdx = (frontBufferIndex == 0 ? (displayBitmapTiles.size() - 1) : (frontBufferIndex - 1));
            gScreenBuffer.drawImageAt(*displayBitmapTiles[prevIdx]->getTileForRenderMode(renderMode), prevIdx * CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        }
    }
    
    // draw a scrubbing line to track the draw progress
    gScreenBuffer.setColour(Colours::grey);
    const int xPosition = frontBufferIndex * CHANNEL_DISPLAY_TILE_WIDTH + frontBackBufferPixelOffset;
    const int yMax = screenBufferImage.getHeight();
    gScreenBuffer.drawLine(xPosition, 0.0f, xPosition, yMax);
}

void ChannelViewCanvas::tick()
{
    if (++frontBackBufferPixelOffset >= CHANNEL_DISPLAY_TILE_WIDTH)
    {
        frontBackBufferPixelOffset = 0;
        
        // advance the index for tile to draw to by one step
        frontBufferIndex += 1;
        if (frontBufferIndex >= displayBitmapTiles.size()) frontBufferIndex = 0;
    }
}

void ChannelViewCanvas::setChannelHeight(float height)
{
    channelHeight = height;
    
    for (int channelId = 0; channelId < channels.size(); ++channelId)
    {
        channels[channelId]->setBounds(30.0f,
                   getHeight() - (channelId / 2 + 1) * (getChannelHeight() * 2) + (channelId % 2 == 0 ? getChannelHeight() : 0),
                   getWidth() - 30.0f,
                   getChannelHeight());
    }
    repaint();
}

float ChannelViewCanvas::getChannelHeight()
{
    return channelHeight;
}

void ChannelViewCanvas::pushPixelValueForChannel(int channel, float rms, float spikeRate, float fft)
{
    channels[channel]->pushSamples(rms, spikeRate, fft);
}

void ChannelViewCanvas::setDisplayedSubprocessor(int subProcessorIdx)
{
    drawableSubprocessorIdx = subProcessorIdx;
}

BitmapRenderTile* const ChannelViewCanvas::getFrontBufferPtr() const
{
    return displayBitmapTiles[frontBufferIndex];
}

int ChannelViewCanvas::getBufferOffsetPosition() const
{
    return frontBackBufferPixelOffset;
}

RenderMode ChannelViewCanvas::getCurrentRenderMode() const
{
    return renderMode;
}

void ChannelViewCanvas::setCurrentRenderMode(RenderMode r)
{
    renderMode = r;
    fullRedraw = true;
    repaint();
}

ColourSchemeId ChannelViewCanvas::getCurrentColourScheme() const
{
    return colourSchemeId;
}

void ChannelViewCanvas::setCurrentColourScheme(ColourSchemeId schemeId)
{
    colourSchemeId = schemeId;
}




# pragma mark - ChannelViewCanvas Constants

const int ChannelViewCanvas::CHANNEL_DISPLAY_MAX_HEIGHT = 2; // more efficient than scaling down, but slightly lossy in definition (cross channel bleed)
const int ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH = 1920;
const int ChannelViewCanvas::CHANNEL_DISPLAY_TILE_WIDTH = 64;
const Colour ChannelViewCanvas::backgroundColour(0, 18, 43);




# pragma mark - BitmapRenderTile -

BitmapRenderTile::BitmapRenderTile(int width, int height, int numChannels)
: width(width)
, height(height)
, numChannels(numChannels)
{
    rms = new Image(Image::RGB, width, height, false);
    spikeRate = new Image(Image::RGB, width, height, false);
    fft = new Image(Image::RGB, width, height, false);
    
    const int subImageHeight = height / numChannels;
    
    for (auto img : {rms.get(), spikeRate.get(), fft.get()})
    {
        Graphics g(*img);
        g.setColour(Colours::black);
        g.fillRect(0, 0, width, height);
        
        channelSubImage.add(new Array<Image>());
        for (int readSite = 0; readSite < numChannels; ++readSite)
        {
            auto subImage = img->getClippedImage(Rectangle<int>(0, height - (readSite + 1) * subImageHeight, width, subImageHeight));
            channelSubImage.getLast()->add(subImage);
        }
    }
    
}

Image* const BitmapRenderTile::getTileForRenderMode(RenderMode mode)
{
    if (mode == RenderMode::RMS) return rms;
    else if (mode == RenderMode::FFT) return fft;
    
    return spikeRate;
}

Image& BitmapRenderTile::getChannelSubImageForRenderMode(int channel, RenderMode mode)
{
    int modeIdx = 0;
    if (mode == RenderMode::RMS)
        modeIdx = 0;
    else if (mode == RenderMode::SPIKE_RATE)
        modeIdx = 1;
    else // if mode is FFT
        modeIdx = 2;
    
    return channelSubImage[modeIdx]->getReference(channel);
}




# pragma mark - ProbeChannelDisplay -

ProbeChannelDisplay::ProbeChannelDisplay(ChannelViewCanvas* channelsView, CanvasOptionsBar* optionsBar, int channelID, float sampleRate)
: channelsView(channelsView)
, optionsBar(optionsBar)
, sampleRate(sampleRate)
, samplesPerPixel(0)
, channelID(channelID)
{
    samplesPerPixel = sampleRate * ProbeViewerCanvas::TRANSPORT_WINDOW_TIMEBASE / float(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH);
}

ProbeChannelDisplay::~ProbeChannelDisplay()
{ }

void ProbeChannelDisplay::paint(Graphics& g)
{ }

// TODO: (kelly) @REFACTOR - this should be rewritten. hangups in other parts of the application could cause problems here
void ProbeChannelDisplay::pxPaint()
{
    // render RMS
    {
        Image::BitmapData bdSubImage(channelsView->getFrontBufferPtr()->getChannelSubImageForRenderMode(channelID, RenderMode::RMS), Image::BitmapData::ReadWriteMode::writeOnly);
        
        float boundSpread = optionsBar->getRMSBoundSpread();
        if (boundSpread == 0) boundSpread = 1;
        const auto val = (rms[rms.size() - channelsView->numPixelUpdates] - optionsBar->getRMSLowBound()) / boundSpread;
        
        Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, channelsView->getCurrentColourScheme());
        
        const int xPix = channelsView->getBufferOffsetPosition();
        for (int yPix = 0; yPix < bdSubImage.height; ++yPix)
        {
            bdSubImage.setPixelColour(xPix, yPix, colour);
        }
    }
    
    // render SPIKE_RATE
    {
        Image::BitmapData bdSubImage(channelsView->getFrontBufferPtr()->getChannelSubImageForRenderMode(channelID, RenderMode::SPIKE_RATE), Image::BitmapData::ReadWriteMode::writeOnly);
        
        float boundSpread = optionsBar->getSpikeRateBoundSpread();
        if (boundSpread == 0) boundSpread = 1;
        const auto val = (spikeRate[spikeRate.size() - channelsView->numPixelUpdates] - optionsBar->getSpikeRateLowBound()) / boundSpread;
        
        Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, channelsView->getCurrentColourScheme());
        
        const int xPix = channelsView->getBufferOffsetPosition();
        for (int yPix = 0; yPix < bdSubImage.height; ++yPix)
        {
            bdSubImage.setPixelColour(xPix, yPix, colour);
        }
    }
    
    // render FFT
    {
        Image::BitmapData bdSubImage(channelsView->getFrontBufferPtr()->getChannelSubImageForRenderMode(channelID, RenderMode::FFT), Image::BitmapData::ReadWriteMode::writeOnly);
        
        float boundSpread = optionsBar->getFFTBoundSpread();
        if (boundSpread == 0) boundSpread = 1;
        const auto val = (fft[fft.size() - channelsView->numPixelUpdates] - optionsBar->getFFTLowBound()) / boundSpread;
        
        Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, channelsView->getCurrentColourScheme());
        
        const int xPix = channelsView->getBufferOffsetPosition();
        for (int yPix = 0; yPix < bdSubImage.height; ++yPix)
        {
            bdSubImage.setPixelColour(xPix, yPix, colour);
        }
    }
    
    if (channelsView->numPixelUpdates == 1)
    {
        rms.clear();
        spikeRate.clear();
        fft.clear();
    }
}

void ProbeChannelDisplay::pushSamples(float rms, float spikeRate, float fft)
{
    // If you reached any of these asserts, some other part of this plugin or open-ephys is
    // causing rendering delays that is causing the sample queue to backup.
    jassert (this->rms.size() < 1024);
    jassert (this->spikeRate.size() < 1024);
    jassert (this->fft.size() < 1024);
    
    this->rms.add(rms);
    this->spikeRate.add(spikeRate);
    this->fft.add(fft);
}

int ProbeChannelDisplay::getChannelId() const
{
    return channelID;
}

void ProbeChannelDisplay::setChannelId(int id)
{
    channelID = id;
}

float ProbeChannelDisplay::getSampleRate()
{
    return sampleRate;
}

int ProbeChannelDisplay::getNumSamplesPerPixel()
{
    return samplesPerPixel;
}
