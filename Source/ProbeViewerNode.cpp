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

#include "ProbeViewerNode.h"

#include "ProbeViewerEditor.h"
#include "ProbeViewerCanvas.h"

using namespace ProbeViewer;

ProbeViewerNode::ProbeViewerNode()
: GenericProcessor ("Probe Viewer")
{
	streamToDraw = -1;
	numStreams = -1;
}

ProbeViewerNode::~ProbeViewerNode()
{ }

AudioProcessorEditor* ProbeViewerNode::createEditor()
{
    editor = std::make_unique<ProbeViewerEditor> (this);
    return editor.get();
}

void ProbeViewerNode::process(AudioBuffer<float>& buffer)
{
	for (int chan = 0; chan < buffer.getNumChannels(); chan++)
    {
		uint16 streamId = continuousChannels[chan]->getStreamId();
		int localId = continuousChannels[chan]->getLocalIndex();
		int globalId = continuousChannels[chan]->getGlobalIndex();
		uint32 nSamples = getNumSamplesInBlock(streamId);

		dataBufferMap[streamId]->addData(buffer, localId, globalId, nSamples);
	}
}

void ProbeViewerNode::updateSettings()
{
    LOGD("Setting num inputs on ProbeViewer to ", getNumInputs());

	ProbeViewerEditor * ed = (ProbeViewerEditor*) getEditor();
	ed->updateStreamSelectorOptions();

	LOGD("Selected Stream ID: ", streamToDraw);
	
	for(auto stream : getDataStreams())
	{
		uint16 streamId = stream->getStreamId();

		if(dataBufferMap.count(streamId) == 0)
		{
			dataBuffers.add(new CircularBuffer(streamId, stream->getSampleRate(), bufferLengthSeconds));
			dataBufferMap[streamId] = dataBuffers.getLast();
		}
		else
		{
			dataBufferMap[streamId]->sampleRate = stream->getSampleRate();
			dataBufferMap[streamId]->prepareToUpdate();
		}

		dataBufferMap[streamId]->setNumChannels(stream->getChannelCount());
	}

	Array<CircularBuffer*> toDelete;

    for (auto dataBuffer : dataBuffers)
    {

        if (dataBuffer->isNeeded)
        {
            dataBuffer->update();
        }
        else
		{
            dataBufferMap.erase(dataBuffer->id);
            toDelete.add(dataBuffer);
        }

    }

    for (auto dataBuffer : toDelete)
    {
        dataBuffers.removeObject(dataBuffer, true);
    }

}

bool ProbeViewerNode::startAcquisition()
{
	((ProbeViewerEditor*) getEditor())->enable();
    return true;
}

bool ProbeViewerNode::stopAcquisition()
{
    ((ProbeViewerEditor*) getEditor())->disable();
    return true;
}

void ProbeViewerNode::setDisplayedStream(int idx)
{
	streamToDraw = idx;
}

uint16 ProbeViewerNode::getDisplayedStream()
{
	return streamToDraw;
}

float ProbeViewerNode::getStreamSampleRate()
{
	if(streamToDraw >= 0)
		return getDataStream(streamToDraw)->getSampleRate();
	else
		return 0.0f;
}

int ProbeViewerNode::getNumStreamChannels()
{
	if(streamToDraw >= 0)
		return getDataStream(streamToDraw)->getChannelCount() ;
	else
		return 0;
}

CircularBuffer* ProbeViewerNode::getCircularBufferPtr()
{
	if(streamToDraw >= 0)
		return dataBufferMap[streamToDraw];
	else
		return nullptr;
}

const float ProbeViewerNode::bufferLengthSeconds = 10.0f;
