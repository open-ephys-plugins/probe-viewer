/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Open Ephys

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

#ifndef __PROBEVIEWERNODE_H__
#define __PROBEVIEWERNODE_H__

#include "ProcessorHeaders.h"

namespace ProbeViewer {

class ProbeViewerNode : public GenericProcessor
{
public:
    ProbeViewerNode();
    virtual ~ProbeViewerNode() override;

    AudioProcessorEditor* createEditor() override;

    void process(AudioSampleBuffer& buffer) override;

    void updateSettings() override;

    bool enable() override;
    bool disable() override;

    void setParameter(int parameterIndex, float newValue) override;

    class CircularBuffer* getCircularBufferPtr() { return dataBuffer; }
//    CriticalSection* getMutex() { return &displayMutex; }

private:
    static const float bufferLengthSeconds;

//    CriticalSection displayMutex;
    ScopedPointer<class CircularBuffer> dataBuffer;

    std::function<int (int)> channelSampleCountPollFunction;

    bool resizeBuffer();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeViewerNode);
};

}

#endif /* __PROBEVIEWERNODE_H__ */
