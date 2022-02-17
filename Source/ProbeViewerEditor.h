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

#ifndef __PROBEVIEWEREDITOR_H__
#define __PROBEVIEWEREDITOR_H__

#include "VisualizerEditorHeaders.h"

namespace ProbeViewer {

class ProbeViewerEditor
    : public VisualizerEditor,
      public ComboBox::Listener
{
public:
    ProbeViewerEditor(GenericProcessor* parentNode);
    virtual ~ProbeViewerEditor() override;

    /** Respond to user's stream sample rate selection */
    void comboBoxChanged(ComboBox *cb) override;

    /** Called by the base calss VisualizerEditor to display the canvas
        when the user chooses to display one

        @see VisualizerEditor::buttonClicked
     */
    virtual Visualizer* createNewCanvas() override;

	void updateStreamSelectorOptions();

	void startAcquisition();
	void stopAcquisition();


private:
    SortedSet<int> inputStreamIds;

    class ProbeViewerNode* probeViewerProcessor;

    std::unique_ptr<Label> streamSelectionLabel;
    std::unique_ptr<ComboBox> streamSelection;

    std::unique_ptr<Label> streamSampleRateLabel;

    bool hasNoInputs;

    void setDrawableStream(int index);
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeViewerEditor);
};

}

#endif /* __PROBEVIEWEREDITOR_H__ */
