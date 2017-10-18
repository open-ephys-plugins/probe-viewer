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
#include "ProbeViewerCanvas.h"

#include "../NeuropixInterface/NeuropixInterface.hpp"
#include "../Utilities/ColourScheme.hpp"

using namespace ProbeViewer;

#pragma mark - ChannelViewCanvas -

ChannelViewCanvas::ChannelViewCanvas(ProbeViewerCanvas* canvas)
: canvas(canvas)
, isDirty(false)
, channelHeight(10)
, screenBufferImage(Image::ARGB, CHANNEL_DISPLAY_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * 394, false)
, numPixelUpdates(0)
, renderMode(RenderMode::RMS)
, frontBackBufferPixelOffset(CHANNEL_DISPLAY_TILE_WIDTH - 1)
{
    
    // get number of tiles that covers *at least* the display width
    int numTiles = ceil(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH / float(ChannelViewCanvas::CHANNEL_DISPLAY_TILE_WIDTH)) + 1;
    
    for (int i = 0; i < numTiles; ++i)
    {
//        auto img = new Image(Image::ARGB, CHANNEL_DISPLAY_TILE_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * 394, false);
//        
//        Graphics gInit(*img);
//        gInit.setColour(Colours::black);
//        gInit.fillRect(0, 0, img->getWidth(), img->getHeight());
        auto tile = new BitmapRenderTile(CHANNEL_DISPLAY_TILE_WIDTH, CHANNEL_DISPLAY_MAX_HEIGHT * 394);
        displayBitmapTiles.add(tile);
    }
}

ChannelViewCanvas::~ChannelViewCanvas()
{ }

void ChannelViewCanvas::paint(Graphics& g)
{
//    Graphics gScreenBuffer(screenBufferImage);
//    gScreenBuffer.setColour(Colours::black);
//    gScreenBuffer.drawRect(0, 0, getWidth(), getHeight());
//    
//    
//    const auto tileImage = getFrontBufferPtr()->getTileForRenderMode(renderMode);
//
//    const float verticalScale = (getChannelHeight() * ChannelViewCanvas::NUM_READ_SITES_FOR_MAX_CHANNELS) / getFrontBufferPtr()->height;
//    const float horizontalScale = getWidth() / float(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH);
//    
//    auto transform = AffineTransform::translation(frontBackBufferPixelOffset * -horizontalScale, 0.0f).scaled(horizontalScale, verticalScale);
//    
//    g.drawImageTransformed(*tileImage, transform);
//    
//    // get each backbuffer (all bitmap tiles before the last tile in the list, in descending order)
//    for (int backBufferIdx = displayBitmapTiles.size() - 2; backBufferIdx >= 0; --backBufferIdx)
//    {
//        // move right by one tile width
//        transform = transform.translated(horizontalScale * ChannelViewCanvas::CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
//        
//        // draw the backbuffer
//        g.drawImageTransformed(*displayBitmapTiles[backBufferIdx]->getTileForRenderMode(renderMode), transform);
//        
//    }
    
    renderTilesToScreenBufferImage();
    
    const float verticalScale = (getChannelHeight() * ChannelViewCanvas::NUM_READ_SITES_FOR_MAX_CHANNELS) / getFrontBufferPtr()->height;
    const float horizontalScale = getWidth() / float(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH);
    const auto transform = AffineTransform::scale(horizontalScale, verticalScale);
//    const auto transform = AffineTransform::scale(0.5, 0.5f);
    g.drawImageTransformed(screenBufferImage, transform);
}

void ChannelViewCanvas::resized()
{
//    for (int readSiteId = 0; readSiteId < readSites.size(); ++readSiteId)
//    {
//        readSites[readSiteId]->setBounds(30.0f,
//                                         getHeight() - (readSiteId / 2 + 1) * (getChannelHeight() * 2) + (readSiteId % 2 == 0 ? getChannelHeight() : 0),
//                                         getWidth() - 30.0f,
//                                         getChannelHeight());
//    }
    repaint();
}

void ChannelViewCanvas::refresh()
{
    if (isDirty.get())
    {
        //ScopedLock lock(imageMutex);
        while (numPixelUpdates > 0)
        {
            // get the front buffer and create a BitmapData accessor object for it
            auto frontBufferPtr = getFrontBufferPtr();
            auto frontTile = frontBufferPtr->getTileForRenderMode(renderMode);
            Image::BitmapData bdFrontBufferBitmap(*frontTile, 0, 0, frontBufferPtr->width, frontBufferPtr->height, Image::BitmapData::ReadWriteMode::writeOnly);
            
            // paint the pixel updates for each channel to the bitmap data
            for (auto channel : channels)
            {
                channel->pxPaint(&bdFrontBufferBitmap);
    //            channel->repaint();
            }
            --numPixelUpdates;
            tick();
        }
        repaint(0, 0, getWidth(), getHeight());
        isDirty.set(false);
    }
}

void ChannelViewCanvas::renderTilesToScreenBufferImage()
{
    Graphics gScreenBuffer(screenBufferImage);
    const auto tileImage = getFrontBufferPtr()->getTileForRenderMode(renderMode);
    
    auto transform = AffineTransform::translation(-frontBackBufferPixelOffset, 0.0f);
    
    gScreenBuffer.drawImageAt(*tileImage, -frontBackBufferPixelOffset, 0.0f);
    
    // get each backbuffer (all bitmap tiles before the last tile in the list, in descending order)
    for (int backBufferIdx = displayBitmapTiles.size() - 2;
         backBufferIdx >= 0;
         --backBufferIdx)
    {
        // move right by one tile width
        transform = transform.translated(ChannelViewCanvas::CHANNEL_DISPLAY_TILE_WIDTH, 0.0f);
        
        // draw the backbuffer
        gScreenBuffer.drawImageTransformed(*displayBitmapTiles[backBufferIdx]->getTileForRenderMode(renderMode), transform);
//        gScreenBuffer.drawImageAt(*displayBitmapTiles[backBufferIdx]->getTileForRenderMode(renderMode), xPosition, 0.0f);
//        xPosition += CHANNEL_DISPLAY_TILE_WIDTH;
        
    }
}

void ChannelViewCanvas::tick()
{
    if (--frontBackBufferPixelOffset < 0)
    {
        frontBackBufferPixelOffset = getFrontBufferPtr()->getTileForRenderMode(renderMode)->getWidth() - 1;
//        auto newBackBuffer = frontBuffer;
//        frontBuffer = backBuffer;
//        backBuffer = newBackBuffer;
        auto prevFrontBuffer = displayBitmapTiles.getFirst();
        displayBitmapTiles.remove(0, false);
        displayBitmapTiles.add(prevFrontBuffer);
    }
}

void ChannelViewCanvas::setChannelHeight(float height)
{
    channelHeight = height;
    
    for (int readSiteId = 0; readSiteId < readSites.size(); ++readSiteId)
    {
        readSites[readSiteId]->setBounds(30.0f,
                   getHeight() - (readSiteId / 2 + 1) * (getChannelHeight() * 2) + (readSiteId % 2 == 0 ? getChannelHeight() : 0),
                   getWidth() - 30.0f,
                   getChannelHeight());
    }
    repaint();
}

float ChannelViewCanvas::getChannelHeight()
{
    return channelHeight;
}

void ChannelViewCanvas::updateRMS(int channel, float value)
{
    channels[channel]->pushSamples(value, 0, 0);
}

void ChannelViewCanvas::setDisplayedSubprocessor(int subProcessorIdx)
{
    drawableSubprocessorIdx = subProcessorIdx;
}

BitmapRenderTile* const ChannelViewCanvas::getFrontBufferPtr() const
{
//    return frontBuffer;
    return displayBitmapTiles[displayBitmapTiles.size() - 1];
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
}




# pragma mark - ChannelViewCanvas Constants

const int ChannelViewCanvas::NUM_READ_SITES_FOR_MAX_CHANNELS = 394;
const int ChannelViewCanvas::CHANNEL_DISPLAY_MAX_HEIGHT = 70; // slightly more than max channel height for full zoom at 1080p
const int ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH = 1920;
const int ChannelViewCanvas::CHANNEL_DISPLAY_TILE_WIDTH = 64;
const Colour ChannelViewCanvas::backgroundColour(0, 18, 43);




# pragma mark - BitmapRenderTile -

BitmapRenderTile::BitmapRenderTile(int width, int height)
: width(width)
, height(height)
{
    rms = new Image(Image::ARGB, width, height, false);
    spikeRate = new Image(Image::ARGB, width, height, false);
    fft = new Image(Image::ARGB, width, height, false);
    
    for (auto img : {rms.get(), spikeRate.get(), fft.get()})
    {
        Graphics g(*img);
        g.setColour(Colours::black);
        g.fillRect(0, 0, width, height);
    }
}

Image* const BitmapRenderTile::getTileForRenderMode(RenderMode mode)
{
    if (mode == RenderMode::RMS) return rms;
    else if (mode == RenderMode::FFT) return fft;
    
    return spikeRate;
}




# pragma mark - ProbeChannelDisplay -

ProbeChannelDisplay::ProbeChannelDisplay(ChannelViewCanvas* channelsView, ChannelState status, int channelID, int readSiteID, float sampleRate)
: channelsView(channelsView)
, sampleRate(sampleRate)
, samplesPerPixel(0)
, channelState(status)
, channelID(channelID)
, readSiteID(readSiteID)
{
    yBitmapPos = (ChannelViewCanvas::NUM_READ_SITES_FOR_MAX_CHANNELS * ChannelViewCanvas::CHANNEL_DISPLAY_MAX_HEIGHT) - (readSiteID + 1) * ChannelViewCanvas::CHANNEL_DISPLAY_MAX_HEIGHT;
    
    if (channelState != ChannelState::reference)
        samplesPerPixel = sampleRate * ProbeViewerCanvas::TRANSPORT_WINDOW_TIMEBASE / float(ChannelViewCanvas::CHANNEL_DISPLAY_WIDTH);
}

ProbeChannelDisplay::~ProbeChannelDisplay()
{ }

void ProbeChannelDisplay::paint(Graphics& g)
{ }

void ProbeChannelDisplay::pxPaint(Image::BitmapData *bitmapData)
{
//    const auto frontBufferPtr = channelsView->getFrontBufferPtr();
//    Image::BitmapData bdFrontBufferBitmap(*frontBufferPtr, 0, 0, frontBufferPtr->getWidth(), frontBufferPtr->getHeight(), Image::BitmapData::ReadWriteMode::writeOnly);
    
    const auto val = (rms[rms.size() - channelsView->numPixelUpdates]) / 250.0f;
    
    Colour colour = ColourScheme::getColourForNormalizedValueInScheme(val, ColourSchemeId::INFERNO);
    
    const int xPix = channelsView->getBufferOffsetPosition();
    const int yUpperBound = yBitmapPos + ChannelViewCanvas::CHANNEL_DISPLAY_MAX_HEIGHT;
    for (int yPix = yBitmapPos; yPix < yUpperBound; ++yPix)
    {
//        bdFrontBufferBitmap.setPixelColour(xPix, yPix, colour);
        bitmapData->setPixelColour(xPix, yPix, colour);
    }
    
    if (channelsView->numPixelUpdates == 1)
        rms.clear();
}

ChannelState ProbeChannelDisplay::getChannelState() const
{
    return channelState;
}

void ProbeChannelDisplay::setChannelState(ChannelState status)
{
    channelState = status;
}

void ProbeChannelDisplay::pushSamples(float rms, float spikeRate, float fft)
{
    if (this->rms.size() > 1024)
    {
//        this->rms.remove(0);
        std::cout << "rms size is growing uncontrollably (" << this->rms.size() << ")" << std::endl;
    }
    
    this->rms.add(rms);
}

int ProbeChannelDisplay::getChannelId() const
{
    return channelID;
}

void ProbeChannelDisplay::setChannelId(int id)
{
    channelID = id;
}

int ProbeChannelDisplay::getReadSiteId() const
{
    return readSiteID;
}

void ProbeChannelDisplay::setReadSiteId(int id)
{
    readSiteID = id;
}

float ProbeChannelDisplay::getSampleRate()
{
    return sampleRate;
}

int ProbeChannelDisplay::getNumSamplesPerPixel()
{
    return samplesPerPixel;
}
