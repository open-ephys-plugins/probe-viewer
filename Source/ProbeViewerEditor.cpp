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

#include "ProbeViewerEditor.h"

#include "ProbeViewerNode.h"
#include "ProbeViewerCanvas.h"

using namespace ProbeViewer;

ProbeViewerEditor::ProbeViewerEditor(GenericProcessor* parentNode)
					: VisualizerEditor(parentNode),
					  hasNoInputs(true)
{
    probeViewerProcessor = (ProbeViewerNode *)parentNode;
    
    tabText = "ProbeViewer";
    
    desiredWidth = 180;
    
    streamSelectionLabel = std::make_unique<Label>("Stream Selection Label", "Display Stream:");
    streamSelectionLabel->setBounds(10, 30, 130, 24);
    addAndMakeVisible(streamSelectionLabel.get());

	streamSelection = std::make_unique<ComboBox>("Stream Selector");
    streamSelection->setBounds(15, 60, 120, 20);
    streamSelection->addListener(this);
    addAndMakeVisible(streamSelection.get());
    
    streamSampleRateLabel = std::make_unique<Label>("Stream Sample Rate Label", "Sample Rate:");
	streamSampleRateLabel->setFont(Font("Fira Code", "SemiBold", 16.0f));
	streamSampleRateLabel->setJustificationType(Justification::centred);
    streamSampleRateLabel->setBounds(10, 90, 160, 24);
    addAndMakeVisible(streamSampleRateLabel.get());
}

ProbeViewerEditor::~ProbeViewerEditor()
{ }


void ProbeViewerEditor::comboBoxChanged(ComboBox* cb)
{
    if (cb == streamSelection.get())
    {
        setDrawableStream(cb->getSelectedId());
    }

	if (canvas != nullptr)
		canvas->update();
}

Visualizer* ProbeViewerEditor::createNewCanvas()
{
    return new ProbeViewerCanvas(probeViewerProcessor);
}

void ProbeViewerEditor::updateStreamSelectorOptions()
{
    // clear out the old data
    inputStreamIds.clear();
    streamSelection->clear(dontSendNotification);
    
	if (probeViewerProcessor->getTotalContinuousChannels() != 0)

	{

		for (int i = 0, len = probeViewerProcessor->getTotalContinuousChannels(); i < len; ++i)
		{
			int streamID = probeViewerProcessor->getContinuousChannel(i)->getStreamId();

			bool success = inputStreamIds.add(streamID);
		}

		int subprocessorToSet = -1;
		if (inputStreamIds.size() > 0)
		{
			subprocessorToSet = inputStreamIds[0];
		}

		for (int i = 0; i < inputStreamIds.size(); ++i)
		{
			streamSelection->addItem(probeViewerProcessor->getDataStream(inputStreamIds[i])->getName(),
										   inputStreamIds[i]);
		}

		if (subprocessorToSet >= 0)
		{
			streamSelection->setSelectedId(subprocessorToSet, dontSendNotification);
		}
		else
		{
			streamSelection->addItem("None", 1);
			streamSelection->setSelectedId(1, dontSendNotification);
		}

		setDrawableStream(subprocessorToSet);
	}
}

void ProbeViewerEditor::setDrawableStream(int index)
{
	if (index >= 0)
	{
		probeViewerProcessor->setDisplayedStream(index);
		float rate = probeViewerProcessor->getStreamSampleRate();

		String sampleRateLabelText = "Sample Rate: ";
		sampleRateLabelText += String(rate);
		streamSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
	}
	else
	{
		probeViewerProcessor->setDisplayedStream(-1);

		String sampleRateLabelText = "Sample Rate: <not available>";
		streamSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
	}
}

void ProbeViewerEditor::startAcquisition()
{
	streamSelection->setEnabled(false);
}

void ProbeViewerEditor::stopAcquisition()
{
	streamSelection->setEnabled(true);
}
