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

		dataBufferMap[streamId]->updateChannelInfo(stream->getContinuousChannels());

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

String ProbeViewerNode::handleConfigMessage(String msg)
{
	
	// message format
	// "<probe_name>;<start_index_1>-<end_index_1>,<region_ID_1>,<hex_color_1>;<start_index_2>-<end_index_2>,...

	// Example:
	// ProbeA;0-69,PT,FF909F;70-97,PVT,FF909F;98-161,-,000000;162-173,-,000000,174-185,SF,90CBED;...

	//LOGC(msg);
	LOGD("Probe Viewer ", getNodeId(), " received message of length ", msg.length());
	
	int firstSemicolon = msg.indexOf(";");
	String probeName;
	String electrodeInfo;

	if (firstSemicolon > -1)
	{
		probeName = msg.substring(0, firstSemicolon);
		electrodeInfo = msg.substring(firstSemicolon + 1);
		LOGD("Probe name: ", probeName);
	}
	else {
		LOGD("No probe name detected.");
		return "No probe name detected.";
	}

	uint16 streamId = 0;

	for (auto stream : dataStreams)
	{
		if (probeName.equalsIgnoreCase(stream->getName()))
			streamId = stream->getStreamId();

	}

	if (streamId == 0)
		return "No matching stream detected.";

	LOGD("Matching stream: ", streamId);

	Array<int> electrodeInds;
	Array<String> regionNames;
	Array<Colour> regionColours;

	StringArray tokens = StringArray::fromTokens(electrodeInfo, ";", "");

	for (auto token : tokens)
	{
		//LOGC(token);

		StringArray rangeInfo = StringArray::fromTokens(token, ",", "");

		int firstElectrode = -1;
		int lastElectrode = -1;
		String regionName = "";
		Colour regionColour = Colours::black;

		int hyphen = rangeInfo[0].indexOf("-");

		if (hyphen > -1)
		{
			firstElectrode = rangeInfo[0].substring(0, hyphen).getIntValue();
			lastElectrode = rangeInfo[0].substring(hyphen + 1).getIntValue();
			//LOGD("First electrode: ", firstElectrode);
			//LOGD("Last electrode: ", lastElectrode);
		}

		if (rangeInfo.size() > 1)
		{
			regionName = rangeInfo[1];
			//LOGD("Region name: ", regionName);
		}

		if (rangeInfo.size() > 2)
		{
			regionColour = Colour::fromString("#FF" + rangeInfo[2].toUpperCase());
			//LOGD("Original color: ", rangeInfo[2]);
			//LOGD("Region colour: ", regionColour.toString());
		}

		if (firstElectrode > -1 && lastElectrode > -1)
		{
			//LOGD("Adding new range.");

			for (int i = firstElectrode; i < lastElectrode + 1; i++)
			{
				electrodeInds.add(i);
				regionNames.add(regionName);
				regionColours.add(regionColour);
			}
		}
	}
		
	if (electrodeInds.size() > 0)
	{
		ProbeViewerEditor* ed = (ProbeViewerEditor*)getEditor();

		ed->setRegions(streamId, electrodeInds, regionNames, regionColours);
	}
	
	return "Success";
}

const float ProbeViewerNode::bufferLengthSeconds = 10.0f;
