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

#ifndef CircularBuffer_hpp
#define CircularBuffer_hpp

#include "VisualizerWindowHeaders.h"

namespace ProbeViewer {

class CircularBuffer
{
public:
    CircularBuffer(int id, float sampleRate, int bufferLengthSec);
    virtual ~CircularBuffer();

    /** Resets buffer*/
    void prepareToUpdate();

    /** Updates buffer settings*/
    void update();

    /** Sets the number of inputs to the buffer*/
    void setNumChannels(int numChans);

    /**
     *  Return the current location of the read point for a specific channel.
     */
    int getChannelReadIndex(int channel) const;

    /**
     *  Return the number of samples that are fresh since the last time
     *  ::clearSamplesReadyForDrawing was called.
     */
    int getNumSamplesReadyForDrawing(int channel) const;

    /**
     *  Returns a value indicating whether or not this CircularBuffer has
     *  fresh samples since the last time ::clearSamplesReadyForDrawing
     *  was called.
     */
    bool hasSamplesReadyForDrawing() const;

    /**
     *  Clears the flag and internal state that holds a set of samples and
     *  advertises them as ready to be drawn with ::hasSamplesReadyForDrawing.
     */
    void clearSamplesReadyForDrawing();

     /** Adds continuous data*/
    void addData(AudioBuffer<float>& buffer, int localChanId, int globalChanId, int nSamples);

    /**
     *  Return a single sample at a specific index and from a specific channel.
     *
     *  Sample-wise reading should only be done after getting this object's
     *  mutex from ::getMutex below.
     */
    float getSample(int sampIdx, int channel) const;

    /**
     *  Return the mutex for locking the internal data buffer during sample read
     *  access.
     *
     *  This is not necessary to use when using ::pushBuffer, as it locks itself.
     */
    CriticalSection* getMutex() { return &dataMutex; }

    int id;
    int bufferLengthSamples;
    float sampleRate;
    bool isNeeded;

private:
    std::unique_ptr<AudioBuffer<float>> dataBuffer;

    Array<int> readIndex;
    Array<int> writeIndex;
	Array<bool> shouldDraw;

    Atomic<int> samplesReadyForDrawing;

    int numChannels;
    int previousSize;

    CriticalSection dataMutex;

};
}

#endif /* CircularBuffer_hpp */
