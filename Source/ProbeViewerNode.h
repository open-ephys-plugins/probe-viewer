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
#include "Utilities/CircularBuffer.hpp"

namespace ProbeViewer {

class ProbeViewerNode : public GenericProcessor
{
public:

    /** Constructor */
    ProbeViewerNode();

    /** Destructor */
    virtual ~ProbeViewerNode() override;

    /** Creates the editor UI */
    AudioProcessorEditor* createEditor() override;

    /** Pushes samples to the data buffer*/
    void process(AudioBuffer<float>& buffer) override;

    /** Updates settings */
    void updateSettings() override;

    /** Updates the displayed stream, then calls updateSettings() */
	void setDisplayedStream(int idx);

    /** Gtes the displayed stream id*/
	uint16 getDisplayedStream();

    /** Returns the sample rate of the currently selected stream*/
	float getStreamSampleRate();

    /** Returns the number of samples in the currently selected stream*/
	int getNumStreamChannels();

    /** Enables the editor */
    bool startAcquisition() override;

    /** Disables the editor*/
    bool stopAcquisition() override;

    /** Returns the buffer for the currently selected stream*/
    CircularBuffer* getCircularBufferPtr();

    /** Responds to config messages with region info */
    String handleConfigMessage(String msg) override;

private:
    static const float bufferLengthSeconds;

    OwnedArray<CircularBuffer> dataBuffers;

    std::map<uint16, CircularBuffer*> dataBufferMap;

	int streamToDraw;
	int numStreams;
	int lastChannelInStream;

	Array<bool> channelsToDraw;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeViewerNode);
};

}

#endif /* __PROBEVIEWERNODE_H__ */
