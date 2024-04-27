/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2017 Open Ephys

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

#ifndef ChannelViewCanvas_hpp
#define ChannelViewCanvas_hpp

#include "VisualizerWindowHeaders.h"

#include "../Utilities/ColourScheme.hpp"

#include "AverageView.hpp"
#include "RollingView.hpp"

namespace ProbeViewer {

class ProbeViewerCanvas;

enum class RenderMode : int
{
    RMS,
    SPIKE_RATE,
    FFT
};


class ChannelViewCanvas : public Component
{
public:

    /** Constructor */
    ChannelViewCanvas(ProbeViewerCanvas*);

    /** Destructor */
    virtual ~ChannelViewCanvas() override { };

    /** Returns number of channels to draw*/
	int getNumChannels() const;

    /** Toggles whether to show / hide average view*/
    void showAverageView(bool);

    /** Updates settings for sub-views*/
	void updateViewSettings();

    /** Called on resize */
    void resized();

    /**
    *  Returns a value describing which RenderMode is currently selected
    *  display on screen (RMS, FFT, or SpikeRate).
    */
    RenderMode getCurrentRenderMode() const;

    /**
     *  Set the RenderMode that should be displayed on screen (FFT, RMS,
     *  or SpikeRate).
     */
    void setCurrentRenderMode(RenderMode r);

    /**
     *  Return a value describing the current user-selected colour
     *  mapping scheme to use for rendering.
     */
    ColourSchemeId getCurrentColourScheme() const;

    /**
     *  Set the current colour mapping that should be used to render
     *  new pixels.
     *
     *  Changing this value does not overwrite old pixels, but will draw
     *  all subsequent pixels with the new mapping.
     */
    void setCurrentColourScheme(ColourSchemeId schemeId);

    class CanvasOptionsBar* optionsBar;

	std::unique_ptr<RollingView> rollingView;
    std::unique_ptr<AverageView> averageView;

private:
	ProbeViewerCanvas* parentCanvas;

    RenderMode renderMode;

    ColourSchemeId colourSchemeId;

};

}



#endif /* ChannelViewCanvas_hpp */
