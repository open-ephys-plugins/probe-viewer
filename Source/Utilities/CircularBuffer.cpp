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

CircularBuffer::CircularBuffer(int id_, float sampleRate_, int bufferLengthInSec) : 
    samplesReadyForDrawing(false),
    id(id_),
    sampleRate(sampleRate_),
    isNeeded(true)
{
    numChannels = 0;
    previousSize = 0;
    bufferLengthSamples = sampleRate * bufferLengthInSec;
    dataBuffer = std::make_unique<AudioSampleBuffer>();
}

CircularBuffer::~CircularBuffer()
{ }


void CircularBuffer::prepareToUpdate()
{
    previousSize = numChannels;
    numChannels = 0;
    isNeeded = false;
} 

void CircularBuffer::updateChannelInfo(Array<ContinuousChannel*> channels)
{
    numChannels = channels.size();
    isNeeded = true;

    channelOrder.clear();

    if (numChannels > 0)
    {

        std::vector<float> depths(numChannels);

        bool allSame = true;
        float last = channels[0]->position.y;

        for (int i = 0; i < numChannels; i++)
        {
            float depth = channels[i]->position.y;

            if (depth != last)
                allSame = false;

            depths[i] = depth;

            last = depth;
        }

        if (allSame)
        {
            LOGD("No depth info found.");
            for (int i = 0; i < numChannels; i++)
                channelOrder.add(i);
        }
        else {
            LOGD("Sorting channels by depth.");
            std::vector<int> V(numChannels);

            std::iota(V.begin(), V.end(), 0); //Initializing
            sort(V.begin(), V.end(), [&](int i, int j) {return depths[i] <= depths[j]; });

            for (int i = 0; i < numChannels; i++)
            {
                // re-order by depth
                channelOrder.add(V[i]);
            }
        }

    }

}


void CircularBuffer::update()
{

    dataBuffer->setSize(numChannels, bufferLengthSamples);
    dataBuffer->clear();

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


void CircularBuffer::addData(AudioBuffer<float>& input, int localChanId, int globalChanId, int numSamples)
{
    samplesReadyForDrawing.set(true);
    ScopedLock dataLock(dataMutex);    

    const int samplesLeft = bufferLengthSamples - writeIndex[localChanId];

    if (numSamples < samplesLeft)
    {
        dataBuffer->copyFrom(channelOrder[localChanId], // dest channel
            writeIndex[localChanId],      // dest startSample
            input,                         // source
            globalChanId,                  // source channel
            0,                             // source start sample
            numSamples);                   // num samples

        writeIndex.set(localChanId, writeIndex[localChanId] + numSamples);
    }
    else
    {
        const int extraSamples = numSamples - samplesLeft;

        dataBuffer->copyFrom(channelOrder[localChanId],
            writeIndex[localChanId],
            input,
            globalChanId,
            0,
            samplesLeft);

        dataBuffer->copyFrom(channelOrder[localChanId],
            0,
            input,
            globalChanId,
            samplesLeft,
            extraSamples);

        writeIndex.set(localChanId, extraSamples);
    }
}

float CircularBuffer::getSample(int sampIdx, int channel) const
{
    int localIdx = sampIdx + readIndex[channel];
    
    if (localIdx >= bufferLengthSamples) localIdx -= bufferLengthSamples;
    
    return dataBuffer->getSample(channel, localIdx);
}
