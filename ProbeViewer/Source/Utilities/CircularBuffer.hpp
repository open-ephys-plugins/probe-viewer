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

#include <VisualizerWindowHeaders.h>

namespace ProbeViewer {

class CircularBuffer
{
public:
    CircularBuffer();
    virtual ~CircularBuffer();
    
    /**
     *  Set the dimensions of the internal buffer memory, channels x samples.
     *
     *  Calling this method may clear the current contents of some or all of
     *  previously stored samples, and will reset the write and read indices to
     *  their initialization state.
     */
    void setSize(int numChannels, int numSamples);
    
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
    
    /**
     *  Read and store a fixed number of samples for all channels in an
     *  AudioSampleBuffer.
     */
    void pushBuffer(AudioSampleBuffer& input, int numSamples);
    
    /**
     *  Read and store all necessary samples from an AudioSampleBuffer.
     *
     *  The channels may have different numbers of samples available, due
     *  to differing sample rates. The second argument is a wrapper for the
     *  source processor's getNumSamples function and is used to check how many
     *  samples should be grabbed for each channel.
     */
    void pushBuffer(AudioSampleBuffer& input, std::function<int (int)> getNumSamples);
    
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
private:
    ScopedPointer<AudioSampleBuffer> dataBuffer;
    
    int bufferLengthSamples;
    
    Array<int> readIndex;
    Array<int> writeIndex;
    
    Atomic<int> samplesReadyForDrawing;
    
    CriticalSection dataMutex;
    
};
}

#endif /* CircularBuffer_hpp */
