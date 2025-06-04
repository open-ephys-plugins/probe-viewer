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

#include "ProbeViewerCanvas.h"
#include "ProbeViewerEditor.h"

using namespace ProbeViewer;

ProbeViewerNode::ProbeViewerNode()
    : GenericProcessor ("Probe Viewer")
{
    streamToDraw = String();
}

ProbeViewerNode::~ProbeViewerNode()
{
}

void ProbeViewerNode::registerParameters()
{
    addIntParameter (Parameter::PROCESSOR_SCOPE, "trigger_line", "Trigger Line", "The TTL trigger line", -1, -1, 15);
    addSelectedStreamParameter (Parameter::PROCESSOR_SCOPE, "display_stream", "Display Stream", "The stream to display", {}, 0, true, false);
}

AudioProcessorEditor* ProbeViewerNode::createEditor()
{
    editor = std::make_unique<ProbeViewerEditor> (this);
    return editor.get();
}

void ProbeViewerNode::process (AudioBuffer<float>& buffer)
{
    for (auto stream : getDataStreams())
    {
        String streamKey = stream->getKey();
        uint16 streamId = stream->getStreamId();
        CircularBuffer* streamBuffer = dataBufferMap[streamKey];
        for (int localId = 0; localId < stream->getChannelCount(); localId++)
        {
            int globalId = stream->getContinuousChannels()[localId]->getGlobalIndex();
            streamBuffer->addData (buffer, localId, globalId, getNumSamplesInBlock (streamId), getFirstSampleNumberForBlock (streamId));
        }
    }

    checkForEvents();
}

void ProbeViewerNode::handleTTLEvent (TTLEventPtr event)
{
    const int eventState = event->getState() ? 1 : 0;
    const int eventLine = event->getLine();
    const int64 sampleNumber = event->getSampleNumber();
    const uint16 streamId = event->getChannelInfo()->getStreamId();

    if (eventState && eventLine == (int) getParameter ("trigger_line")->getValue())
    {
        dataBufferMap[getDataStream (streamId)->getKey()]->setTrigger (sampleNumber);
    }
}

void ProbeViewerNode::updateSettings()
{
    for (auto buffer : dataBuffers)
    {
        buffer->prepareToUpdate();
    }

    for (auto stream : getDataStreams())
    {
        String streamKey = stream->getKey();

        if (dataBufferMap.count (streamKey) == 0)
        {
            dataBuffers.add (new CircularBuffer (streamKey, stream->getSampleRate(), bufferLengthSeconds));
            dataBufferMap[streamKey] = dataBuffers.getLast();
        }
        else
        {
            dataBufferMap[streamKey]->sampleRate = stream->getSampleRate();
        }

        dataBufferMap[streamKey]->updateChannelInfo (stream->getContinuousChannels());
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
            dataBufferMap.erase (dataBuffer->key);
            toDelete.add (dataBuffer);
        }
    }

    for (auto dataBuffer : toDelete)
    {
        dataBuffers.removeObject (dataBuffer, true);
    }
}

void ProbeViewerNode::parameterValueChanged (Parameter* param)
{
    if (param->getName() == "display_stream")
    {
        String streamKey = param->getValueAsString();
        if (auto stream = getDataStream (streamKey))
            setDisplayedStream (streamKey);
        else
            setDisplayedStream (String());
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

void ProbeViewerNode::setDisplayedStream (const String& streamKey)
{
    streamToDraw = streamKey;
    ((ProbeViewerEditor*) getEditor())->displayStreamChanged();
}

String ProbeViewerNode::getDisplayedStream() const
{
    return streamToDraw;
}

float ProbeViewerNode::getStreamSampleRate()
{
    if (streamToDraw.isNotEmpty())
        return getDataStream (streamToDraw)->getSampleRate();
    else
        return 0.0f;
}

int ProbeViewerNode::getNumStreamChannels()
{
    if (streamToDraw.isNotEmpty())
        return getDataStream (streamToDraw)->getChannelCount();
    else
        return 0;
}

CircularBuffer* ProbeViewerNode::getCircularBufferPtr()
{
    if (streamToDraw.isNotEmpty() && dataBufferMap.count (streamToDraw) > 0)
        return dataBufferMap[streamToDraw];
    else
        return nullptr;
}

String ProbeViewerNode::handleConfigMessage (const String& msg)
{
    // message format
    // "<probe_name>;<start_index_1>-<end_index_1>,<region_ID_1>,<hex_color_1>;<start_index_2>-<end_index_2>,...

    // Example:
    // ProbeA;0-69,PT,FF909F;70-97,PVT,FF909F;98-161,-,000000;162-173,-,000000,174-185,SF,90CBED;...

    //LOGD(msg);
    LOGD ("Probe Viewer ", getNodeId(), " received message of length ", msg.length());

    int firstSemicolon = msg.indexOf (";");
    String probeName;
    String electrodeInfo;

    if (firstSemicolon > -1)
    {
        probeName = msg.substring (0, firstSemicolon);
        electrodeInfo = msg.substring (firstSemicolon + 1);
        LOGD ("Probe name: ", probeName);
    }
    else
    {
        LOGD ("No probe name detected.");
        return "No probe name detected.";
    }

    Array<String> streamKeys;

    for (auto stream : dataStreams)
    {
        String streamName = stream->getName();

        if (streamName.endsWith ("-AP"))
        {
            streamName = streamName.substring (0, streamName.length() - 3);
        }
        else if (streamName.endsWith ("-LFP"))
        {
            streamName = streamName.substring (0, streamName.length() - 4);
        }

        if (probeName.equalsIgnoreCase (streamName))
            streamKeys.add (stream->getKey());
    }

    LOGD ("Number of matching streams: ", streamKeys.size());

    if (streamKeys.size() == 0)
    {
        return "No matching stream detected.";
    }

    Array<int> electrodeInds;
    Array<String> regionNames;
    Array<Colour> regionColours;

    StringArray tokens = StringArray::fromTokens (electrodeInfo, ";", "");

    for (auto token : tokens)
    {
        //LOGC(token);

        StringArray rangeInfo = StringArray::fromTokens (token, ",", "");

        int firstElectrode = -1;
        int lastElectrode = -1;
        String regionName = "";
        Colour regionColour = Colours::black;

        int hyphen = rangeInfo[0].indexOf ("-");

        if (hyphen > -1)
        {
            firstElectrode = rangeInfo[0].substring (0, hyphen).getIntValue();
            lastElectrode = rangeInfo[0].substring (hyphen + 1).getIntValue();
            //LOGD("First electrode: ", firstElectrode);
            //LOGD("Last electrode: ", lastElectrode);
        }

        if (rangeInfo.size() > 1)
        {
            regionName = rangeInfo[1];

            if (regionName.startsWith ("SSp"))
                regionName = "SSp";
            //LOGD("Region name: ", regionName);
        }

        if (rangeInfo.size() > 2)
        {
            regionColour = Colour::fromString ("#FF" + rangeInfo[2].toUpperCase());
            //LOGD("Original color: ", rangeInfo[2]);
            //LOGD("Region colour: ", regionColour.toString());
        }

        if (firstElectrode > -1 && lastElectrode > -1)
        {
            //LOGD("Adding new range.");

            for (int i = firstElectrode; i < lastElectrode + 1; i++)
            {
                electrodeInds.add (i);
                regionNames.add (regionName);
                regionColours.add (regionColour);
            }
        }
    }

    if (electrodeInds.size() > 0)
    {
        ProbeViewerEditor* ed = (ProbeViewerEditor*) getEditor();

        for (auto streamKey : streamKeys)
            ed->setRegions (streamKey, electrodeInds, regionNames, regionColours);
    }

    return "Success";
}

const float ProbeViewerNode::bufferLengthSeconds = 10.0f;
