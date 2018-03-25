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

#include "CircularBuffer.hpp"

using namespace ProbeViewer;

CircularBuffer::CircularBuffer()
: samplesReadyForDrawing(false)
{
    dataBuffer = new AudioSampleBuffer(8, 100);
    readIndex.insertMultiple(0, 0, 8);
    writeIndex.insertMultiple(0, 0, 8);
}

CircularBuffer::~CircularBuffer()
{ }

void CircularBuffer::setSize(int numChannels, int numSamples, Array<bool> shouldDraw_)
{
    bufferLengthSamples = numSamples;
    dataBuffer->setSize(numChannels, numSamples);

	shouldDraw = shouldDraw_;
    
    readIndex.clear();
    readIndex.insertMultiple(0, 0, numChannels);
    
    writeIndex.clear();
    writeIndex.insertMultiple(0, 0, numChannels);
}

int CircularBuffer::getChannelReadIndex(int channel) const
{
    return readIndex[channel];
}

int CircularBuffer::getNumSamplesReadyForDrawing(int channel) const
{
    int numSamples = writeIndex[channel] - readIndex[channel];
    if (numSamples < 0)
    {
        numSamples = bufferLengthSamples - 1 - readIndex[channel] + writeIndex[channel] - 1;
    }
    
    return numSamples;
}

bool CircularBuffer::hasSamplesReadyForDrawing() const
{
    return samplesReadyForDrawing.get();
}

void CircularBuffer::clearSamplesReadyForDrawing()
{
    samplesReadyForDrawing.set(false);
    
    for (int i = 0; i < readIndex.size(); ++i)
    {
        readIndex.set(i, writeIndex[i]);
    }
}

void CircularBuffer::pushBuffer(AudioSampleBuffer& input, int numSamples)
{
    samplesReadyForDrawing.set(true);
    ScopedLock dataLock(dataMutex);

	int channelIndex = -1;
    
    for (int channel = 0; channel < input.getNumChannels(); ++channel)
    {
		if (shouldDraw[channel])
		{

			channelIndex++; 

			const int samplesLeft = bufferLengthSamples - writeIndex[channelIndex];

			if (numSamples < samplesLeft)
			{
				dataBuffer->copyFrom(channel,
					writeIndex[channelIndex],
					input,
					channelIndex,
					0,
					numSamples);

				writeIndex.set(channelIndex, writeIndex[channelIndex] + numSamples);
			}
			else
			{
				const int extraSamples = numSamples - samplesLeft;

				dataBuffer->copyFrom(channel,
					writeIndex[channelIndex],
					input,
					channelIndex,
					0,
					samplesLeft);

				dataBuffer->copyFrom(channel,
					0,
					input,
					channelIndex,
					samplesLeft,
					extraSamples);

				writeIndex.set(channelIndex, extraSamples);
			}
		}
    }
}

void CircularBuffer::pushBuffer(AudioSampleBuffer& input, std::function<int (int)> getNumSamples)
{
    samplesReadyForDrawing.set(true);
    
    ScopedLock dataLock(dataMutex);
    
    for (int channel = 0; channel < input.getNumChannels(); ++channel)
    {
        const int samplesLeft = bufferLengthSamples - writeIndex[channel];
        const int numSamples = getNumSamples(channel);
        
        if (numSamples < samplesLeft)
        {
            dataBuffer->copyFrom(channel,
                                 writeIndex[channel],
                                 input,
                                 channel,
                                 0,
                                 numSamples);
            
            writeIndex.set(channel, writeIndex[channel] + numSamples);
        }
        else
        {
            const int extraSamples = numSamples - samplesLeft;
            
            dataBuffer->copyFrom(channel,
                                 writeIndex[channel],
                                 input,
                                 channel,
                                 0,
                                 samplesLeft);
            
            dataBuffer->copyFrom(channel,
                                 0,
                                 input,
                                 channel,
                                 samplesLeft,
                                 extraSamples);
            
            writeIndex.set(channel, extraSamples);
        }
    }
}

float CircularBuffer::getSample(int sampIdx, int channel) const
{
    int localIdx = sampIdx + readIndex[channel];
    
    if (localIdx >= bufferLengthSamples) localIdx -= bufferLengthSamples;
    
    return dataBuffer->getSample(channel, localIdx);
}
