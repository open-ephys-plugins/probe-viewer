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

#include "../ChannelBrowser/ChannelBrowser.hpp"
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
, screenBufferImage(Image::RGB, CHANNEL_DISPLAY_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * 384, false)
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
            // paint the pixel updates for each channel to the bitmap data
            for (auto channel : channels)
            {
                channel->pxPaint();
            }
            --numPixelUpdates;
            
            tick();
        }
        repaint();
        isDirty.set(false);
    }
}

void ChannelViewCanvas::updateViewSettings()
{
    numChannels = canvas->getNumChannels();

    displayBitmapTiles.clear();

    frontBufferIndex = 0;
    frontBackBufferPixelOffset = 0;

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
            gScreenBuffer.drawImageAt(*displayBitmapTiles[backBufferIdx]->getTile(), backBufferIdx * CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        }
        
        fullRedraw = false;
    }
    else
    {
        const bool shouldRedrawLastBufferToo = true;
        
        gScreenBuffer.drawImageAt(*getFrontBufferPtr()->getTile(), frontBufferIndex * CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        
        if (shouldRedrawLastBufferToo)
        {
            const int prevIdx = (frontBufferIndex == 0 ? (displayBitmapTiles.size() - 1) : (frontBufferIndex - 1));
            gScreenBuffer.drawImageAt(*displayBitmapTiles[prevIdx]->getTile(), prevIdx * CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        }
    }
    
    // draw a scrubbing line to track the draw progress
    gScreenBuffer.setColour(Colours::yellow);
    const int xPosition = frontBufferIndex * CHANNEL_DISPLAY_TILE_WIDTH + frontBackBufferPixelOffset;
    const int yMax = screenBufferImage.getHeight();
    gScreenBuffer.fillRect(xPosition, 0, 1, yMax);

    //std::cout << "Front buffer pixel offset: " << frontBackBufferPixelOffset << ", Cursor: " << xPosition << std::endl;
    //std::cout << "Ratio: " << float(xPosition) / float(frontBufferIndex) << std::endl;

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

void ChannelViewCanvas::pushPixelValueForChannel(int channel, float value)
{
    channels[channel]->pushSample(value);
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
    renderImage = Image(Image::RGB, width, height, false);
    
    const int subImageHeight = height / numChannels;
    
    Graphics g(renderImage);
    g.setColour(Colours::black);
    g.fillRect(0, 0, width, height);
    
    for (int readSite = 0; readSite < numChannels; ++readSite)
    {
        auto subImage = renderImage.getClippedImage(Rectangle<int>(0, height - (readSite + 1) * subImageHeight, width, subImageHeight));
        channelSubImage.add(subImage);
    }
    
}

Image* const BitmapRenderTile::getTile()
{
    return &renderImage;
}

Image BitmapRenderTile::getChannelSubImage(int channel)
{
    return channelSubImage[channel];
}




# pragma mark - ProbeChannelDisplay -

ProbeChannelDisplay::ProbeChannelDisplay(ChannelViewCanvas* channelsView_, 
    CanvasOptionsBar* optionsBar_, 
    int channelID_, 
    float sampleRate_)
: channelsView(channelsView_)
, optionsBar(optionsBar_)
, sampleRate(sampleRate_)
, channelID(channelID_)
{
    samplesPerPixel = sampleRate * ProbeViewerCanvas::TRANSPORT_WINDOW_TIMEBASE 
                      / float(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH);
}

ProbeChannelDisplay::~ProbeChannelDisplay()
{ }

void ProbeChannelDisplay::pxPaint()
{
    RenderMode rm = channelsView->getCurrentRenderMode();

    Image bdSubImage(channelsView->getFrontBufferPtr()->getChannelSubImage(channelID));

    // render RMS
    if(rm == RenderMode::RMS)
    {

        float boundSpread = optionsBar->getRMSBoundSpread();
        if (boundSpread == 0) boundSpread = 1;
        const auto val = (samples[samples.size() - channelsView->numPixelUpdates] - optionsBar->getRMSLowBound()) / boundSpread;
        
        Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, channelsView->getCurrentColourScheme());
        
        const int xPix = channelsView->getBufferOffsetPosition();

        for (int yPix = 0; yPix < bdSubImage.getHeight(); ++yPix)
        {
            bdSubImage.setPixelAt(xPix, yPix, colour);
        }
    }
    
    // render SPIKE_RATE
    else if(rm == RenderMode::SPIKE_RATE)
    {        
        float boundSpread = optionsBar->getSpikeRateBoundSpread();
        if (boundSpread == 0) boundSpread = 1;
        const auto val = (samples[samples.size() - channelsView->numPixelUpdates] - optionsBar->getSpikeRateLowBound()) / boundSpread;
        
        Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, channelsView->getCurrentColourScheme());
        
        const int xPix = channelsView->getBufferOffsetPosition();
        for (int yPix = 0; yPix < bdSubImage.getHeight(); ++yPix)
        {
            bdSubImage.setPixelAt(xPix, yPix, colour);
        }
    }
    
    // render FFT
    else
    {
        float boundSpread = optionsBar->getFFTBoundSpread();
        if (boundSpread == 0) boundSpread = 1;
        const auto val = (samples[samples.size() - channelsView->numPixelUpdates] - optionsBar->getFFTLowBound()) / boundSpread;
        
        Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, channelsView->getCurrentColourScheme());
        
        const int xPix = channelsView->getBufferOffsetPosition();
        for (int yPix = 0; yPix < bdSubImage.getHeight(); ++yPix)
        {
            bdSubImage.setPixelAt(xPix, yPix, colour);
        }
    }
    
    if (channelsView->numPixelUpdates == 1)
    {
        samples.clear();
    }
}

void ProbeChannelDisplay::pushSample(float sample)
{
    // If you reached this assert, some other part of this plugin or open-ephys is
    // causing rendering delays that is causing the sample queue to backup.
    jassert (samples.size() < 1024);
    this->samples.add(sample);
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
