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

#include "ProbeViewerCanvas.h"

#include "ProbeViewerNode.h"
#include "ProbeViewerEditor.h"

#include "NeuropixInterface/NeuropixInterface.hpp"
#include "ChannelViewCanvas/ChannelViewCanvas.hpp"
#include "Utilities/CircularBuffer.hpp"

using namespace ProbeViewer;

#pragma mark - ProbeViewerCanvas -

ProbeViewerCanvas::ProbeViewerCanvas(ProbeViewerNode* processor_)
: pvProcessor(processor_)
, numChannels(0)
{
    dataBuffer = pvProcessor->getCircularBufferPtr();
    
    interface = new NeuropixInterface(this);
    addAndMakeVisible(interface);
    
    channelsView = new ChannelViewCanvas(this);
    
    viewport = new ProbeViewerViewport(this, channelsView);
    viewport->setViewedComponent(channelsView, false);
    viewport->setScrollBarsShown(false, false);
    addAndMakeVisible(viewport);
    
}

ProbeViewerCanvas::~ProbeViewerCanvas()
{ }

void ProbeViewerCanvas::refreshState()
{
    
}

void ProbeViewerCanvas::update()
{
    numChannels = jmax(pvProcessor->getNumInputs(), 1);
    
    channelsView->readSites.clear();
    channelsView->channels.clear();
    partialBufferCache.clear();
    
    int referenceNodeOffsetCount = 0;
    for (int i = 0; i < NeuropixInterface::MAX_NUM_CHANNELS; ++i)
    {
        if (NeuropixInterface::refNodes.contains(i + referenceNodeOffsetCount + 1))
        {
            channelsView->readSites.add(new ProbeChannelDisplay(channelsView, ChannelState::reference, -1, i + referenceNodeOffsetCount, 0));
            ++referenceNodeOffsetCount;
        }
        
        float sampleRate = 0;
        if (pvProcessor->getNumInputs() > 0) sampleRate = pvProcessor->getDataChannel(i)->getSampleRate();
        else sampleRate = 30000;
        
        auto channelDisplay = new ProbeChannelDisplay(channelsView, ChannelState::enabled, i, i + referenceNodeOffsetCount, sampleRate);
        
        channelsView->readSites.add(channelDisplay);
        channelsView->channels.add(channelDisplay);
        partialBufferCache.add(new Array<float>());
    }
    
}

void ProbeViewerCanvas::refresh()
{
    updateScreenBuffers();
    
    channelsView->refresh();
}

void ProbeViewerCanvas::beginAnimation()
{
    std::cout << "Beginning animation." << std::endl;
    
    startCallbacks();
}

void ProbeViewerCanvas::endAnimation()
{
    std::cout << "Ending animation." << std::endl;
    
    stopCallbacks();
}

void ProbeViewerCanvas::paint(Graphics& g)
{ }

void ProbeViewerCanvas::resized()
{
    interface->setBounds(0, 0, 200, getHeight());
    channelsView->setBounds(0, 0, viewport->getWidth(), channelsView->getChannelHeight() * channelsView->readSites.size());
    viewport->setBounds(interface->getRight(), 4, getWidth() - interface->getWidth(), getHeight() - 6);
}

void ProbeViewerCanvas::setNumChannels(int numChannels)
{
    this->numChannels = numChannels;
}

int ProbeViewerCanvas::getNumChannels()
{
    return numChannels;
}

void ProbeViewerCanvas::setChannelHeight(float height)
{
    channelsView->setChannelHeight(height);
}

float ProbeViewerCanvas::getChannelHeight()
{
    return channelsView->getChannelHeight();
}

float ProbeViewerCanvas::getChannelSampleRate(int channel)
{
    return channelsView->channels[channel]->getSampleRate();
}

void ProbeViewerCanvas::setDrawableSubprocessor(int idx)
{
    channelsView->setDisplayedSubprocessor(idx);
}

ProbeViewerViewport* ProbeViewerCanvas::getViewportPtr()
{
    return viewport;
}

ChannelViewCanvas* ProbeViewerCanvas::getChannelViewCanvasPtr()
{
    return channelsView;
}

// TODO: (kelly) this should be implemented differently, as is it will shift the array after every pop
float ProbeViewerCanvas::popFrontCachedSampleForChannel(int channel)
{
    float val = 0;
    if (!partialBufferCache[channel]->isEmpty())
    {
        val = partialBufferCache[channel]->getReference(0);
        partialBufferCache[channel]->remove(0);
    }
    return val;
}

// anonymous static helper function to encapsulate repeated logic in updateScreenBuffers() below
namespace {
    void processSample(float&& val, int& index, float& min, float& max, Array<float>& samplesBuffer)
    {
        samplesBuffer.set(index, val);
        
        if (index == 0)
        {
            min = val;
            max = val;
        }
        else
        {
            if (val > max) max = val;
            if (val < min) min = val;
        }
    }
}

void ProbeViewerCanvas::updateScreenBuffers()
{
    if (dataBuffer->hasSamplesReadyForDrawing())
    {
//        std::cout << Time::getCurrentTime().toString(false, true) << " There are " << dataBuffer->getNumSamplesReadyForDrawing(0) << " samples ready to draw" << std::endl;
        
        int numTicks = 0;
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const int numSamplesToRead = dataBuffer->getNumSamplesReadyForDrawing(channel);
            const int numCachedSamples = getNumCachedSamples(channel);
            const int samplesPerPixel = channelsView->channels[channel]->getNumSamplesPerPixel();
            const int numPixelsToCreate = (numCachedSamples + numSamplesToRead) / samplesPerPixel;
            
            if (numTicks < numPixelsToCreate) numTicks = numPixelsToCreate;
            
            int sampleBufferIndex = 0;
            for (int pix = 0; pix < numPixelsToCreate; ++pix)
            {
                float min = 0;
                float max = 0;
                Array<float> samples;
                samples.resize(samplesPerPixel);
                
                // find min, max
                if (pix == 0 && numCachedSamples > 0)
                {
                    for (int cachedSampIdx = 0; cachedSampIdx < numCachedSamples; ++cachedSampIdx)
                    {
//                        float val = popFrontCachedSampleForChannel(channel);
//                        samples.set(cachedSampIdx, val);
//                        
//                        if (cachedSampIdx == 0)
//                        {
//                            min = val;
//                            max = val;
//                        }
//                        else{
//                            if (val > max) max = val;
//                            if (val < min) min = val;
//                        }
                        processSample(popFrontCachedSampleForChannel(channel),
                                      cachedSampIdx,
                                      min,
                                      max,
                                      samples);
                    }
                }
                
                // find min, max
                for (int sampIdx = (pix == 0 && numCachedSamples > 0 ? numCachedSamples : 0); sampIdx < samplesPerPixel; ++sampIdx)
                {
//                    float val = dataBuffer->getSample(sampleBufferIndex++, channel);
//                    samples.set(sampIdx, val);
//                    
//                    if (sampIdx == 0)
//                    {
//                        min = val;
//                        max = val;
//                    }
//                    else{
//                        if (val > max) max = val;
//                        if (val < min) min = val;
//                    }
                    processSample(dataBuffer->getSample(sampleBufferIndex, channel),
                                  sampIdx,
                                  min,
                                  max,
                                  samples);
                    ++sampleBufferIndex;
                }
                
                float median = (max + min) / 2.0f;
                float rms = 0;
                
                for (int sampIdx = 0; sampIdx < samplesPerPixel; ++sampIdx)
                {
                    const float medianOffsetVal = samples[sampIdx] - median;
                    
                    rms += (medianOffsetVal * medianOffsetVal);
                }
                
                rms = sqrtf(rms / samplesPerPixel);
                channelsView->updateRMS(channel, rms);
                
                //if (channel == 0)
    //            std::cout << channel << " => " << rms << std::endl;
            }
            
            for (int sampIdx = sampleBufferIndex; sampIdx < numSamplesToRead; ++sampIdx)
            {
                partialBufferCache[channel]->add(dataBuffer->getSample(sampleBufferIndex, channel));
            }
        }
        
        channelsView->numPixelUpdates = numTicks;
        
        channelsView->isDirty.set(true);
        dataBuffer->clearSamplesReadyForDrawing();
        repaint(0, 0, getWidth(), getHeight());
    }
    else
    {
//        std::cout << Time::getCurrentTime().toString(false, true) << " No samples to draw" << std::endl;
    }
}

int ProbeViewerCanvas::getNumCachedSamples(int channel)
{
    return partialBufferCache[channel]->size();
}




# pragma mark - ProbeViewerCanvas Constants
const float ProbeViewerCanvas::TRANSPORT_WINDOW_TIMEBASE = 10.0f;




# pragma mark - ProbeViewerViewport -
ProbeViewerViewport::ProbeViewerViewport(ProbeViewerCanvas* canvas, ChannelViewCanvas* channelsView)
: Viewport()
, canvas(canvas)
, channelsView(channelsView)
{ }

ProbeViewerViewport::~ProbeViewerViewport()
{ }

void ProbeViewerViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{
//    float viewPosition = getViewPositionY() / float(channelsView->getHeight());
//    setViewPosition(0, viewPosition);
}
