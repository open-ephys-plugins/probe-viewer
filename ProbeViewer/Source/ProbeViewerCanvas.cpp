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

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif

#include "ProbeViewerCanvas.h"

#include "ProbeViewerNode.h"
#include "ProbeViewerEditor.h"

#include "NeuropixInterface/NeuropixInterface.hpp"
#include "ChannelViewCanvas/ChannelViewCanvas.hpp"
#include "ChannelViewCanvas/CanvasOptionsBar.hpp"
#include "TimeScale/ProbeViewerTimeScale.hpp"
#include "Utilities/CircularBuffer.hpp"

using namespace ProbeViewer;

#pragma mark - ProbeViewerCanvas -

ProbeViewerCanvas::ProbeViewerCanvas(ProbeViewerNode* processor_)
: pvProcessor(processor_)
, fft_cfg(kiss_fftr_alloc(ProbeViewerCanvas::FFT_SIZE, false, 0, 0))
, numChannels(0)
, numSamplesToChunk(1)
{
    dataBuffer = pvProcessor->getCircularBufferPtr();
    
    interface = new NeuropixInterface(this);
    addAndMakeVisible(interface);
    
    timeScale = new ProbeViewerTimeScale(ProbeViewerCanvas::TRANSPORT_WINDOW_TIMEBASE, 0.5f);
    addAndMakeVisible(timeScale);
    
    channelsView = new ChannelViewCanvas(this);
    
    optionsBar = new CanvasOptionsBar(channelsView);
    addAndMakeVisible(optionsBar);
    optionsBar->setFFTParams(ProbeViewerCanvas::FFT_SIZE, ProbeViewerCanvas::FFT_TARGET_SAMPLE_RATE);
    channelsView->optionsBar = optionsBar;
    
    viewport = new ProbeViewerViewport(this, channelsView);
    viewport->setViewedComponent(channelsView, false);
    viewport->setScrollBarsShown(false, false);
    addAndMakeVisible(viewport);
    
    // init the fft input/output containers
    for (int i = 0; i < ProbeViewerCanvas::FFT_SIZE; ++i)
    {
        fftInput.push_back(0.0f);
    }
    
    setBufferedToImage(true);
}

ProbeViewerCanvas::~ProbeViewerCanvas()
{
    free(fft_cfg);
}

void ProbeViewerCanvas::refreshState()
{
    
}

void ProbeViewerCanvas::update()
{
    // TODO: (kelly) the options bar for the FFT rendering is only configured to support
    //               one global sample rate. if several sampleRates are represented at
    //               this will still work, but the center frequency combobox options will
    //               only be accurate for one of the sampleRates - currently, the first
    //               non-zero sample rate encountered
    numChannels = jmax(pvProcessor->getNumInputs(), 0);
    setNumChannels(numChannels);
    
    channelsView->readSites.clear();
    channelsView->channels.clear();
    partialBufferCache.clear();
    channelFFTSampleBuffer.clear();
    inputDownsamplingIndex.clear();
    
    if (numChannels != interface->getNumActiveChannels())
        interface->setNumActiveChannels(numChannels);
    
    int referenceNodeOffsetCount = 0;
    float globalSampleRate = 0;
    for (int i = 0; i < NeuropixInterface::MAX_NUM_CHANNELS; ++i)
    {
        if (NeuropixInterface::refNodes.contains(i + referenceNodeOffsetCount + 1))
        {
            channelsView->readSites.add(new ProbeChannelDisplay(channelsView, optionsBar, ChannelState::reference, -1, i + referenceNodeOffsetCount, 0));
            ++referenceNodeOffsetCount;
        }
        
        float sampleRate = 0;
        const int procInputs = pvProcessor->getNumInputs();
        if (procInputs > 0 && procInputs < getNumChannels()) sampleRate = pvProcessor->getDataChannel(i)->getSampleRate();
        else sampleRate = 30000;
        
        auto channelDisplay = new ProbeChannelDisplay(channelsView, optionsBar, ChannelState::enabled, i, i + referenceNodeOffsetCount, sampleRate);
        
        channelsView->readSites.add(channelDisplay);
        channelsView->channels.add(channelDisplay);
        partialBufferCache.add(new Array<float>());
        
        channelFFTSampleBuffer.add(new FFTSampleCacheBuffer(ProbeViewerCanvas::FFT_SIZE));
        inputDownsamplingIndex.push_back(0);
        
        if (globalSampleRate == 0) globalSampleRate = sampleRate;
    }
    
    // see note above ^
    numSamplesToChunk = int(globalSampleRate / ProbeViewerCanvas::FFT_TARGET_SAMPLE_RATE);
    
    optionsBar->setFFTParams(ProbeViewerCanvas::FFT_SIZE, ProbeViewerCanvas::FFT_TARGET_SAMPLE_RATE);
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
    timeScale->setBounds(0, 0, getWidth(), 30);
    optionsBar->setBounds(0, getHeight() - 30, getWidth(), 30);
    
    interface->setBounds(0, timeScale->getBottom(), 200, getHeight() - timeScale->getHeight() - optionsBar->getHeight());
    timeScale->setMarginOffset(interface->getWidth());
    optionsBar->setMarginOffset(interface->getWidth());
    
    channelsView->setBounds(0, 0, viewport->getWidth(), channelsView->getChannelHeight() * channelsView->readSites.size());
    viewport->setBounds(interface->getRight(), timeScale->getBottom() + 2, getWidth() - interface->getWidth(), getHeight() - timeScale->getHeight() - optionsBar->getHeight() - 4);
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

void ProbeViewerCanvas::updateScreenBuffers()
{
    if (dataBuffer->hasSamplesReadyForDrawing())
    {
        ScopedLock drawLock(*dataBuffer->getMutex());
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
                
                // find min, max for cached samples
                if (pix == 0 && numCachedSamples > 0)
                {
                    for (int cachedSampIdx = 0; cachedSampIdx < numCachedSamples; ++cachedSampIdx)
                    {
                        const auto val = popFrontCachedSampleForChannel(channel);
                        samples.set(cachedSampIdx, val);
                        
                        if (cachedSampIdx == 0)
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
                
                
                // find min, max for new buffer samples
                for (int sampIdx = (pix == 0 && numCachedSamples > 0 ? numCachedSamples : 0); sampIdx < samplesPerPixel; ++sampIdx)
                {
                    const auto val = dataBuffer->getSample(sampleBufferIndex, channel);
                    samples.set(sampIdx, val);
                    
                    if (sampIdx == 0)
                    {
                        min = val;
                        max = val;
                    }
                    else
                    {
                        if (val > max) max = val;
                        if (val < min) min = val;
                    }
                    ++sampleBufferIndex;
                }
                
                float median = (max + min) / 2.0f;
                float rms = 0;
                float spikeRate = 0;
                int numSpikesInPixel = 0;
                
                const float spikeRateThreshold = optionsBar->getSpikeRateThreshold();
                
                for (int sampIdx = 0; sampIdx < samplesPerPixel; ++sampIdx)
                {
                    const float medianOffsetVal = samples[sampIdx] - median;
                    
                    rms += (medianOffsetVal * medianOffsetVal);
  
                    if (medianOffsetVal < spikeRateThreshold) numSpikesInPixel++;
                    
//                    channelFFTSampleBuffer[channel].push_back(medianOffsetVal / 500);
                    if (inputDownsamplingIndex[channel]++ == 0)
                    {
                        channelFFTSampleBuffer[channel]->pushSample(medianOffsetVal / 500.0f);
                    }
                    else if (inputDownsamplingIndex[channel] >= numSamplesToChunk)
                    {
                        inputDownsamplingIndex[channel] = 0;
                    }
                }
                
                
                //
                //
                //              PERFORM FFT FOR THIS PIXEL AND CHANNEL
                //
                //
                for (int sampleIdx = 0; sampleIdx < ProbeViewerCanvas::FFT_SIZE; ++sampleIdx)
                {
                    fftInput[sampleIdx] = fftWindow[sampleIdx] * channelFFTSampleBuffer[channel]->readSample(sampleIdx);
                }
                
                kiss_fftr(fft_cfg, fftInput.data(), fftOutput);
                //
                //
                //          END FFT FOR PIXEL AND CHANNEL
                //
                //
                
                const int bin = optionsBar->getFFTCenterFrequencyBin();
                const float fftValueDb = 20 * log10((fftOutput[bin].r * fftOutput[bin].r + fftOutput[bin].i * fftOutput[bin].i) * 2 / ProbeViewerCanvas::FFT_SIZE);
                
                rms = sqrtf(rms / samplesPerPixel);
                
                spikeRate = numSpikesInPixel / (samplesPerPixel / getChannelSampleRate(channel));
                
                channelsView->pushPixelValueForChannel(channel, rms, spikeRate, fftValueDb);
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
// load the fftWindow with a Hanning window
const std::vector<float> ProbeViewerCanvas::fftWindow = []() -> std::vector<float> {
    std::vector<float> window;
    
    for (int i = 0; i < ProbeViewerCanvas::FFT_SIZE; ++i)
    {
        window.push_back(0.5 * (1 - cos((2 * M_PI * i)/(ProbeViewerCanvas::FFT_SIZE - 1))));
    }
    
    return window;
}();




# pragma mark - ProbeViewerCanvas::FFTSampleCacheBuffer -

ProbeViewerCanvas::FFTSampleCacheBuffer::FFTSampleCacheBuffer(int size)
: writeIdx(0)
, readIdx(1)
, bufferSize(size)
{
    buffer.resize(size);
}

ProbeViewerCanvas::FFTSampleCacheBuffer::~FFTSampleCacheBuffer()
{ }

void ProbeViewerCanvas::FFTSampleCacheBuffer::resize(const int size)
{
    bufferSize = size;
    buffer.clear();
    buffer = std::vector<float>(size, 0.0f);
    
    writeIdx = 0;
    readIdx = 1;
}

namespace {
    void incrementIndices(int & writeIdx, int & readIdx, int bufferSize)
    {
        writeIdx = readIdx++;
        
        if (readIdx >= bufferSize)
        {
            readIdx = 0;
        }
        
        jassert(writeIdx < bufferSize);
        jassert(readIdx < bufferSize);
    }
}

void ProbeViewerCanvas::FFTSampleCacheBuffer::pushSample(const float sample)
{
    buffer[writeIdx] = sample;
    incrementIndices(writeIdx, readIdx, bufferSize);
}

float ProbeViewerCanvas::FFTSampleCacheBuffer::readSample(int index) const
{
    jassert(index < bufferSize);
    
    index += readIdx;
    
    if (index >= bufferSize)
    {
        index -= bufferSize;
    }
    
    jassert(index < bufferSize);
    
    return buffer[index];
}




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
