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
    
    subprocessorSelection = new ComboBox("Subprocessor sample rate");
    subprocessorSelection->setBounds(10, 30, 50, 22);
    subprocessorSelection->addListener(this);
    addAndMakeVisible(subprocessorSelection);
    
    subprocessorSelectionLabel = new Label("subprocessorSelectionLabel", "Display Subproc.");
    subprocessorSelectionLabel->setBounds(subprocessorSelection->getRight(), subprocessorSelection->getY(), 100, 20);
    addAndMakeVisible(subprocessorSelectionLabel);
    
    subprocessorSampleRateLabel = new Label("subprocessorSampleRateLabel", "Sample Rate:");
    subprocessorSampleRateLabel->setFont(Font(Font::getDefaultSerifFontName(), 14.0f, Font::plain));
    subprocessorSampleRateLabel->setBounds(subprocessorSelection->getX(), subprocessorSelection->getBottom() + 10, 200, 40);
    addAndMakeVisible(subprocessorSampleRateLabel);
}

ProbeViewerEditor::~ProbeViewerEditor()
{ }


void ProbeViewerEditor::comboBoxChanged(ComboBox* cb)
{
    if (cb == subprocessorSelection)
    {
        setCanvasDrawableSubprocessor(cb->getSelectedId());
    }

	if (canvas != nullptr)
		canvas->update();
}

Visualizer* ProbeViewerEditor::createNewCanvas()
{
    return new ProbeViewerCanvas(probeViewerProcessor);
}

void ProbeViewerEditor::updateSubprocessorSelectorOptions()
{
    // clear out the old data
    inputStreamIds.clear();
    inputSampleRates.clear();
    subprocessorSelection->clear(dontSendNotification);
    
	if (probeViewerProcessor->getTotalContinuousChannels() != 0)

	{

		for (int i = 0, len = probeViewerProcessor->getTotalContinuousChannels(); i < len; ++i)
		{
			int streamID = probeViewerProcessor->getContinuousChannel(i)->getStreamId();

			bool success = inputStreamIds.add(streamID);

			if (success) inputSampleRates.set(streamID, probeViewerProcessor->getContinuousChannel(i)->getSampleRate());
		}

		int subprocessorToSet = -1;
		if (inputStreamIds.size() > 0)
		{
			subprocessorToSet = inputStreamIds[0];
		}

		for (int i = 0; i < inputStreamIds.size(); ++i)
		{
			subprocessorSelection->addItem(probeViewerProcessor->getDataStream(inputStreamIds[i])->getName(),
										   inputStreamIds[i]);
		}

		if (subprocessorToSet >= 0)
		{
			subprocessorSelection->setSelectedId(subprocessorToSet, dontSendNotification);

			String sampleRateLabelText = "Sample Rate: ";
			sampleRateLabelText += String(inputSampleRates[subprocessorToSet]);

			subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
			setCanvasDrawableSubprocessor(subprocessorToSet);
		}
		else
		{
			subprocessorSelection->addItem("None", 1);
			subprocessorSelection->setSelectedId(1, dontSendNotification);

			String sampleRateLabelText = "Sample Rate: <not available>";
			subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
			setCanvasDrawableSubprocessor(-1);
		}
	}
}

void ProbeViewerEditor::setCanvasDrawableSubprocessor(int index)
{
	if (canvas)
	{
		if (index >= 0)
		{
			((ProbeViewerCanvas *)canvas.get())->setDrawableSubprocessor(index);
			float rate = probeViewerProcessor->getStreamSampleRate();

			String sampleRateLabelText = "Sample Rate: ";
			sampleRateLabelText += String(rate);
			subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);

			std::cout << sampleRateLabelText << std::endl;
		}
		else
		{
			((ProbeViewerCanvas *)canvas.get())->setDrawableSubprocessor(-1);
		}

	}
}

void ProbeViewerEditor::startAcquisition()
{
	subprocessorSelection->setEnabled(false);
}

void ProbeViewerEditor::stopAcquisition()
{
	subprocessorSelection->setEnabled(true);
}
