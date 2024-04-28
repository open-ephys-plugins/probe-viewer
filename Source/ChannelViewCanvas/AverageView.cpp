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

#include "AverageView.hpp"

#include "ChannelViewCanvas.hpp"
#include "CanvasOptionsBar.hpp"
#include "../Utilities/CircularBuffer.hpp"
#include "../ProbeViewerNode.h"

using namespace ProbeViewer;

const int AVERAGE_VIEW_WIDTH = 300;

AverageView::AverageView(ChannelViewCanvas* canvas_, ProbeViewerNode* node_) :
    canvas(canvas_),
    node(node_),
    numChannels(1),
    numTrials(0),
    channelHeight(10),
    screenBufferImage(Image::RGB, AVERAGE_VIEW_WIDTH, 1, true)
{

}


void AverageView::updateViewSettings()
{
    
    numChannels = canvas->getNumChannels();

    if (numChannels == 0)
        numChannels = 1;

    screenBufferImage = Image(Image::RGB, AVERAGE_VIEW_WIDTH, numChannels * 2, true);

	screenBuffer.setSize(numChannels, AVERAGE_VIEW_WIDTH);
    screenBuffer.clear();
   
    cacheBuffer.setSize(numChannels, 1000);
    cacheBuffer.clear();

    numTrials = 1;
    
    pixelIndex.clear();
    pixelIndex.insertMultiple(0, 0, numChannels);

    numCachedSamples.clear();
    numCachedSamples.insertMultiple(0, 0, numChannels);
}

void AverageView::comboBoxChanged(ComboBox* c)
{
    updateImage = true;

    if (c == canvas->optionsBar->renderModeSelection)
    {
        screenBuffer.clear();
        numTrials = 1;
    }

    repaint();
}


void AverageView::paint(Graphics& g)
{

    const float verticalScale =  float(channelHeight * numChannels) / (numChannels * 2);
    const float horizontalScale = getWidth() / float(AVERAGE_VIEW_WIDTH);

    const auto transform = AffineTransform::scale(horizontalScale, verticalScale).followedBy(AffineTransform::verticalFlip(getHeight()));

    if (updateImage)
    {
        int zeroMarker = int(preWindow / (preWindow + postWindow) * float(AVERAGE_VIEW_WIDTH));
        
        float boundSpread = canvas->optionsBar->getRMSBoundSpread();
        const float lowerBound = canvas->optionsBar->getRMSLowBound();
        
        if (boundSpread == 0) boundSpread = 1;
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
			for (int pixel = 0; pixel < AVERAGE_VIEW_WIDTH; ++pixel)
			{
                if (pixel == zeroMarker)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        screenBufferImage.setPixelAt(pixel, channel * 2 + i, Colours::yellow);
                    }
                }
                else {
                    const float value = screenBuffer.getSample(channel, pixel) / numTrials;
                    const float normValue = (value - lowerBound) / boundSpread;

                    const Colour colour = ColourScheme::getColourForNormalizedValueInScheme(normValue, canvas->getCurrentColourScheme());

                    for (int i = 0; i < 2; i++)
                    {
                        screenBufferImage.setPixelAt(pixel, channel * 2 + i, colour);
                    }
                    
                }
                

			}
        }
        
        updateImage = false;
    }

    g.drawImageTransformed(screenBufferImage, transform);

}

void AverageView::setChannelHeight(float height)
{
    channelHeight = height;

    repaint();
}

void AverageView::setSampleRate(float sampleRate_)
{
    sampleRate = sampleRate_;

    samplesPerPixel = sampleRate * (preWindow + postWindow)
        / float(AVERAGE_VIEW_WIDTH);
}


void AverageView::setWindow(float pre, float post)
{
	preWindow = pre;
	postWindow = post;

	samplesPerPixel = sampleRate * (preWindow + postWindow)
		/ float(AVERAGE_VIEW_WIDTH);

    updateImage = true;

    screenBuffer.clear();
    numTrials = 1;
    repaint();
}

void AverageView::mouseDown(const MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        PopupMenu m;

        m.addItem(98, "TTL Trigger Line", false);
        m.addItem(99, "None", true, triggerLine == -1);
        m.addItem(1, "TTL 1", true, triggerLine == 0);
        m.addItem(2, "TTL 2", true, triggerLine == 1);
		m.addItem(3, "TTL 3", true, triggerLine == 2);
		m.addItem(4, "TTL 4", true, triggerLine == 3);
		m.addItem(5, "TTL 5", true, triggerLine == 4);
		m.addItem(6, "TTL 6", true, triggerLine == 5);
		m.addItem(7, "TTL 7", true, triggerLine == 6);
		m.addItem(8, "TTL 8", true, triggerLine == 7);
        
		const int result = m.show();

        if (result == 0)
            return;

        if (result == 99)
            triggerLine = -1;
        else
            triggerLine = result - 1;

        node->getParameter("trigger_line")->setNextValue(triggerLine);
		setWindow(preWindow, postWindow);
		
	}
}

void AverageView::fillFromBuffer(CircularBuffer* dataBuffer)
{

    // grab samples from start of window up to current sample
    int64 windowStartSampleNumber = dataBuffer->triggerSampleNumber - int(preWindow * sampleRate);
    int64 windowEndSampleNumber = dataBuffer->triggerSampleNumber + int(postWindow * sampleRate);
    int64 currentSampleNumber = dataBuffer->latestSampleNumber;

    RenderMode modeId = canvas->getCurrentRenderMode();

    for (int channel = 0; channel < numChannels; ++channel)
    {

        int64 numSamplesAvailable;
        int sampleBufferIndex;

        if (pixelIndex[channel] == 0) // just received trigger
        {
            if (channel == 0)
                numTrials += 1;
            
            numSamplesAvailable = currentSampleNumber - windowStartSampleNumber;
            sampleBufferIndex = dataBuffer->getNumSamplesReadyForDrawing(channel) - numSamplesAvailable;
        }
        else {
            numSamplesAvailable = dataBuffer->getNumSamplesReadyForDrawing(channel) + numCachedSamples[channel];
            sampleBufferIndex = 0;
        }

        const int numPixelsToCreate = numSamplesAvailable / samplesPerPixel;

        if (numPixelsToCreate == 0)
        {
            break;
        }

        for (int pix = 0; pix < numPixelsToCreate; ++pix)
        {
            float min = 0;
            float max = 0;
            Array<float> samples;
            samples.resize(samplesPerPixel);

            // find min, max for cached samples
            if (pix == 0 && numCachedSamples[channel] > 0)
            {
                for (int cachedSampIdx = 0; cachedSampIdx < numCachedSamples[channel]; ++cachedSampIdx)
                {
                    const auto val = cacheBuffer.getSample(channel, cachedSampIdx);
                    samples.set(cachedSampIdx, val);

                    if (cachedSampIdx == 0)
                    {
                        min = val;
                        max = val;
                    }
                    else
                    {
                        if (val > max)
                            max = val;
                        if (val < min)
                            min = val;
                    }
                }
            }

            // find min, max for new buffer samples
            for (int sampIdx = (pix == 0 && numCachedSamples[channel] > 0 ? numCachedSamples[channel] : 0); sampIdx < samplesPerPixel; ++sampIdx)
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
                    if (val > max)
                        max = val;
                    if (val < min)
                        min = val;
                }
                ++sampleBufferIndex;
            }

            float median = (max + min) / 2.0f;
            float rms = 0;
            float spikeRate = 0;
            int numSpikesInPixel = 0;

            const float spikeRateThreshold = canvas->optionsBar->getSpikeRateThreshold();

            for (int sampIdx = 0; sampIdx < samplesPerPixel; ++sampIdx)
            {
                const float medianOffsetVal = samples[sampIdx] - median;

                if (modeId == RenderMode::RMS) // RMS
                {
                    rms += (medianOffsetVal * medianOffsetVal);
                }
                else if (modeId == RenderMode::SPIKE_RATE) // Spike Rate
                {
                    if (medianOffsetVal < spikeRateThreshold)
                        numSpikesInPixel++;
                }
            }

            if (modeId == RenderMode::RMS)
            {
                rms = sqrtf(rms / samplesPerPixel);

               // if (pixelIndex[channel] < 5)
               //     std::cout << int(rms) << " ";

                screenBuffer.addSample(channel, pixelIndex[channel], rms);

            }
            else if (modeId == RenderMode::SPIKE_RATE)
            {
                spikeRate = numSpikesInPixel / (samplesPerPixel / sampleRate);
                screenBuffer.addSample(channel, pixelIndex[channel], spikeRate);
            }

            pixelIndex.set(channel, pixelIndex[channel] + 1);

            int newIndex = pixelIndex[channel];

            if (pixelIndex[channel] == AVERAGE_VIEW_WIDTH)
            {
                pixelIndex.set(channel, 0);
                
                if (channel == numChannels - 1)
                {
                    dataBuffer->triggered = false;
                    
                    updateImage = true;
                    repaint();
                }
                break;
            }
        }

        //std::cout << std::endl;

        //for (int sampIdx = sampleBufferIndex; sampIdx < numSamplesAvailable; ++sampIdx)
       // {
        //    cacheBuffer.setSample(channel, sampIdx, dataBuffer->getSample(sampleBufferIndex, channel));
        //}

    }


    
}