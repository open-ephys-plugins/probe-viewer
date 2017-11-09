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
    dataBuffer->pushBuffer(b, channelSampleCountPollFunction);
}

void ProbeViewerNode::updateSettings()
{
    std::cout << "Setting num inputs on ProbeViewer to " << getNumInputs() << std::endl;
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

bool ProbeViewerNode::resizeBuffer()
{
    int nSamples = (int) getSampleRate() * bufferLengthSeconds;
    int nInputs = getNumInputs();
    
    std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;
    
    if (nSamples > 0 && nInputs > 0)
    {
        dataBuffer->setSize(nInputs, nSamples);
        return true;
    }
    
    return false;
}

const float ProbeViewerNode::bufferLengthSeconds = 10.0f;
