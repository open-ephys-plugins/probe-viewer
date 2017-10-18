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

ProbeViewerEditor::ProbeViewerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors)
: VisualizerEditor(parentNode, useDefaultParameterEditors)
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

void ProbeViewerEditor::buttonClicked(Button* button)
{
    // duplicate default VisualizerEditor behavior, except...
    if (canvas == nullptr)
    {
        canvas = createNewCanvas();
        
        // initialize the subprocessor sample rate filtering before canvas updates
        updateSubprocessorSelectorOptions();
        ((ProbeViewerCanvas*)canvas.get())->setDrawableSubprocessor(*(inputSubprocessorIndices.begin() + (subprocessorSelection->getSelectedId() - 1)));
        
        canvas->update();
        
        if (isPlaying)
            canvas->beginAnimation();
    }
    
    // resume default behavior
    VisualizerEditor::buttonClicked(button);
}

void ProbeViewerEditor::buttonEvent(Button* button)
{
    // nothing to do here
}

void ProbeViewerEditor::comboBoxChanged(ComboBox* cb)
{
    if (cb == subprocessorSelection)
    {
        setCanvasDrawableSubprocessor(cb->getSelectedId() - 1);
    }
}

Visualizer* ProbeViewerEditor::createNewCanvas()
{
    canvas = new ProbeViewerCanvas(probeViewerProcessor);
    updateSubprocessorSelectorOptions();
    return canvas;
}

void ProbeViewerEditor::updateSubprocessorSelectorOptions()
{
    // clear out the old data
    inputSubprocessorIndices.clear();
    inputSampleRates.clear();
    subprocessorSelection->clear(dontSendNotification);
    
    for (int i = 0, len = probeViewerProcessor->getTotalDataChannels(); i < len; ++i)
    {
        int subProcessorIdx = probeViewerProcessor->getDataChannel(i)->getSubProcessorIdx();
        
        bool success = inputSubprocessorIndices.add(subProcessorIdx);
        
        if (success) inputSampleRates.set(subProcessorIdx, probeViewerProcessor->getDataChannel(i)->getSampleRate());
        
        //        if (success) std::cout << "\t\tadding subprocessor index " << subProcessorIdx << std::endl;
    }
    
    int subprocessorToSet = -1;
    if (inputSubprocessorIndices.size() > 0)
    {
        subprocessorToSet = 0;
    }
    
    for (int i = 0; i < inputSubprocessorIndices.size(); ++i)
    {
        subprocessorSelection->addItem (String (*(inputSubprocessorIndices.begin() + i)), i + 1);
    }
    
    if (subprocessorToSet >= 0)
    {
        subprocessorSelection->setSelectedId(subprocessorToSet + 1, dontSendNotification);
        
        String sampleRateLabelText = "Sample Rate: ";
        sampleRateLabelText += String(inputSampleRates[*(inputSubprocessorIndices.begin()+subprocessorToSet)]);
        
        subprocessorSampleRateLabel->setText(sampleRateLabelText, dontSendNotification);
        setCanvasDrawableSubprocessor(subprocessorToSet);
    }
}

void ProbeViewerEditor::setCanvasDrawableSubprocessor(int index)
{
    if (canvas)
    {
        ((ProbeViewerCanvas *) canvas.get())->setDrawableSubprocessor(*(inputSubprocessorIndices.begin() + index));
    }
}
