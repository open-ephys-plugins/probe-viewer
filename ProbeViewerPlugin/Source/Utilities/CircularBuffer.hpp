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
    
    void setSize(int numChannels, int numSamples);
    
    int getChannelReadIndex(int channel) const;
    int getNumSamplesReadyForDrawing(int channel) const;
    
    bool hasSamplesReadyForDrawing() const;
    void clearSamplesReadyForDrawing();
    
    void pushBuffer(AudioSampleBuffer& input, int numSamples);
    void pushBuffer(AudioSampleBuffer& input, std::function<int (int)> getNumSamples);
    
    float getSample(int sampIdx, int channel) const;
    
//    CriticalSection* getMutex() { return &dataMutex; }
private:
    ScopedPointer<AudioSampleBuffer> dataBuffer;
    
    Array<int> readIndex;
    Array<int> writeIndex;
    
    Atomic<int> samplesReadyForDrawing;
    
    CriticalSection dataMutex;
    
};
}

#endif /* CircularBuffer_hpp */
