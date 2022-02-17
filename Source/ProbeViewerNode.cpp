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

#include "Utilities/CircularBuffer.hpp"

using namespace ProbeViewer;

ProbeViewerNode::ProbeViewerNode()
: GenericProcessor ("Probe Viewer")
{
    setProcessorType(Plugin::Processor::SINK);
    dataBuffer = new CircularBuffer;
    
    // bind the get samples function
    using namespace std::placeholders;
    channelSampleCountPollFunction = std::bind(&ProbeViewerNode::getNumSamples, this, _1);

	streamToDraw = 0;
	numStreams = -1;
	lastChannelInStream = 0;
}

ProbeViewerNode::~ProbeViewerNode()
{ }

AudioProcessorEditor* ProbeViewerNode::createEditor()
{
    editor = std::make_unique<ProbeViewerEditor> (this);
    return editor.get();
}

void ProbeViewerNode::process(AudioSampleBuffer& b)
{
	const int nSamples = getNumSamples(lastChannelInStream);

	dataBuffer->pushBuffer(b, nSamples);
}

void ProbeViewerNode::updateSettings()
{
    LOGD("Setting num inputs on ProbeViewer to ", getNumInputs());

	lastChannelInStream = 0;

	channelsToDraw.clear();

	// update the editor's subprocessor selection display, only if there's a mismatch in # of subprocessors
	if (numStreams != getNumDataStreams())
	{
		numStreams = getNumDataStreams();
		ProbeViewerEditor * ed = (ProbeViewerEditor*) getEditor();
		ed->updateStreamSelectorOptions();
	}

	LOGD("Selected Stream ID: ", streamToDraw);

	for (int i = 0; i < getNumInputs(); i++)
	{
		auto chan = continuousChannels[i];
		int channelStream = chan->getStreamId();

		if (channelStream == streamToDraw)
		{
			//std::cout << "Found a match" << std::endl;
			channelsToDraw.add(true);
			lastChannelInStream = i;
		}
		else {
			channelsToDraw.add(false);
		}
	}

	//std::cout << "Resizing buffer!" << std::endl;
	resizeBuffer();

}

bool ProbeViewerNode::startAcquisition()
{
    if (resizeBuffer())
    {
        auto editor = (ProbeViewerEditor*) getEditor();
        
        editor->enable();
        return true;
    }
    
    return false;
}

bool ProbeViewerNode::stopAcquisition()
{
    ((ProbeViewerEditor*) getEditor())->disable();
    return true;
}

void ProbeViewerNode::setDisplayedStream(int idx)
{
	streamToDraw = idx;
	updateSettings();
}

float ProbeViewerNode::getStreamSampleRate()
{
	return getDataStream(streamToDraw)->getSampleRate();
}

int ProbeViewerNode::getNumStreamChannels()
{
	return getDataStream(streamToDraw)->getChannelCount() ;
}

bool ProbeViewerNode::resizeBuffer()
{
	int nSamples = (int) getStreamSampleRate() * bufferLengthSeconds;
	int nInputs = getNumStreamChannels();
    
    LOGD("Resizing buffer. Samples: ", nSamples, ", Inputs: ", nInputs);
    
    if (nSamples > 0 && nInputs > 0)
    {
        dataBuffer->setSize(nInputs, nSamples, channelsToDraw);
        return true;
    }
    
    return false;
}

const float ProbeViewerNode::bufferLengthSeconds = 10.0f;
