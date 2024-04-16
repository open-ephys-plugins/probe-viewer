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

#include "ChannelViewCanvas.hpp"

#include "AverageView.hpp"
#include "RollingView.hpp"
#include "../ProbeViewerCanvas.h"

using namespace ProbeViewer;


ChannelViewCanvas::ChannelViewCanvas(class ProbeViewerCanvas* canvas)
    : parentCanvas(canvas),
      renderMode(RenderMode::RMS),
      colourSchemeId(ColourSchemeId::INFERNO)
{
    rollingView = std::make_unique<RollingView>(this);
    averageView = std::make_unique<AverageView>(this);

    addAndMakeVisible(rollingView.get());
    addAndMakeVisible(averageView.get());
}

void ChannelViewCanvas::resized()
{
    rollingView->setBounds(0, 0, getWidth() - 300, getHeight());
    averageView->setBounds(getWidth() - 290, 0, 290, getHeight());   
}


int ChannelViewCanvas::getNumChannels() const
{
    return parentCanvas->getNumChannels();
}

void ChannelViewCanvas::updateViewSettings()
{
    rollingView->updateViewSettings();
    averageView->updateViewSettings();
}


RenderMode ChannelViewCanvas::getCurrentRenderMode() const
{
    return renderMode;
}

void ChannelViewCanvas::setCurrentRenderMode(RenderMode r)
{
    renderMode = r;
    rollingView->fullRedraw = true;
    repaint();
}

ColourSchemeId ChannelViewCanvas::getCurrentColourScheme() const
{
    return colourSchemeId;
}

void ChannelViewCanvas::setCurrentColourScheme(ColourSchemeId schemeId)
{
    colourSchemeId = schemeId;
}
