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

#include "ProbeViewerCanvas.h"
#include "ProbeViewerNode.h"

using namespace ProbeViewer;

ProbeViewerEditor::ProbeViewerEditor (GenericProcessor* parentNode)
    : VisualizerEditor (parentNode, "ProbeViewer"),
      hasNoInputs (true)
{
    probeViewerProcessor = (ProbeViewerNode*) parentNode;

    desiredWidth = 175;
    addSelectedStreamParameterEditor (Parameter::PROCESSOR_SCOPE, "display_stream", 15, 30);
    auto* streamEditor = getParameterEditor ("display_stream");
    streamEditor->setLayout (ParameterEditor::nameOnTop);
    streamEditor->setSize (140, 40);

    streamSampleRateLabel = std::make_unique<Label> ("Stream Sample Rate Label", "Sample Rate:");
    streamSampleRateLabel->setFont (FontOptions ("Inter", "Medium", 14.0f));
    streamSampleRateLabel->setBounds (15, 90, 140, 24);
    addAndMakeVisible (streamSampleRateLabel.get());
}

ProbeViewerEditor::~ProbeViewerEditor()
{
}

Visualizer* ProbeViewerEditor::createNewCanvas()
{
    return new ProbeViewerCanvas (probeViewerProcessor);
}

void ProbeViewerEditor::updateSettings()
{
    if (canvas != nullptr)
    {
        static_cast<ProbeViewerCanvas*> (canvas.get())->updateChannelBrowsers();
    }
}

void ProbeViewerEditor::displayStreamChanged()
{
    if (canvas != nullptr)
    {
        static_cast<ProbeViewerCanvas*> (canvas.get())->updateSettings();
    }

    float rate = probeViewerProcessor->getStreamSampleRate();

    String sampleRateLabelText = "Sample Rate: ";

    if (rate > 0)
        sampleRateLabelText += String (rate);
    else
        sampleRateLabelText += "<NA>";

    streamSampleRateLabel->setText (sampleRateLabelText, dontSendNotification);
}

/** Sets custom depths and regions */
void ProbeViewerEditor::setRegions (String streamKey, Array<int>& electrodeInds, Array<String>& regionNames, Array<Colour>& regionColours)
{
    if (canvas != nullptr)
    {
        static_cast<ProbeViewerCanvas*> (canvas.get())->setRegions (streamKey, electrodeInds, regionNames, regionColours);
    }
}