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
    setProcessorType(PROCESSOR_TYPE_SINK);
    dataBuffer = new CircularBuffer;
    
    // bind the get samples function
    using namespace std::placeholders;
    channelSampleCountPollFunction = std::bind(&ProbeViewerNode::getNumSamples, this, _1);

	subprocessorToDraw = 0;
	numSubprocessors = -1;
	numChannelsInSubprocessor = 0;
	lastChannelInSubprocessor = 0;
}

ProbeViewerNode::~ProbeViewerNode()
{ }

AudioProcessorEditor* ProbeViewerNode::createEditor()
{
    editor = new ProbeViewerEditor (this, true);
    return editor;
}

void ProbeViewerNode::process(AudioSampleBuffer& b)
{
	const int nSamples = getNumSamples(lastChannelInSubprocessor);

	dataBuffer->pushBuffer(b, nSamples);
}

void ProbeViewerNode::updateSettings()
{
    std::cout << "Setting num inputs on ProbeViewer to " << getNumInputs() << std::endl;

	int numChannelsInSubprocessor = 0;
	int totalSubprocessors = 0;
	int currentSubprocessor = -1;
	lastChannelInSubprocessor = 0;

	channelsToDraw.clear();

	for (int i = 0; i < getNumInputs(); i++)
	{
		int channelSubprocessor = getDataChannel(i)->getSubProcessorIdx();

		if (currentSubprocessor != channelSubprocessor)
		{
			totalSubprocessors++;
			currentSubprocessor = channelSubprocessor;
		}

		if (channelSubprocessor == subprocessorToDraw)
		{
			numChannelsInSubprocessor++;
			subprocessorSampleRate = getDataChannel(i)->getSampleRate();
			channelsToDraw.add(true);
			lastChannelInSubprocessor = i;
		}
		else {
			channelsToDraw.add(false);
		}
	}

	// update the editor's subprocessor selection display, only if there's a mismatch in # of subprocessors
	if (numSubprocessors != totalSubprocessors)
	{
		ProbeViewerEditor * ed = (ProbeViewerEditor*) getEditor();
		ed->updateSubprocessorSelectorOptions();
		numSubprocessors = totalSubprocessors;
	}

}

bool ProbeViewerNode::enable()
{
    if (resizeBuffer())
    {
        auto editor = (ProbeViewerEditor*) getEditor();
        
        editor->enable();
        return true;
    }
    
    return false;
}

bool ProbeViewerNode::disable()
{
    ((ProbeViewerEditor*) getEditor())->disable();
    return true;
}

void ProbeViewerNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    
    // Sets Parameters array for processor
    parameters[parameterIndex]->setValue(newValue, currentChannel);
    
    ProbeViewerEditor* ed = (ProbeViewerEditor*) getEditor();
    if(ed->canvas != 0)
        ed->canvas->setParameter (parameterIndex, newValue);
}

void ProbeViewerNode::setDisplayedSubprocessor(int idx)
{
	subprocessorToDraw = idx;
	updateSettings();
}

float ProbeViewerNode::getSubprocessorSampleRate()
{
	return subprocessorSampleRate;
}

bool ProbeViewerNode::resizeBuffer()
{
    int nSamples = (int) getSampleRate() * bufferLengthSeconds;
	int nInputs = numChannelsInSubprocessor;
    
    std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;
    
    if (nSamples > 0 && nInputs > 0)
    {
        dataBuffer->setSize(nInputs, nSamples, channelsToDraw);
        return true;
    }
    
    return false;
}

const float ProbeViewerNode::bufferLengthSeconds = 10.0f;
