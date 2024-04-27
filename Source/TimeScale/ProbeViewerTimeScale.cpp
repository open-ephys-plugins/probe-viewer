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

#include "ProbeViewerTimeScale.hpp"

using namespace ProbeViewer;

TimeScale::TimeScale() :
    font("Default", 16, Font::plain)
{

}

void TimeScale::paint(Graphics& g)
{
    g.setColour(Colour(35, 35, 35));
    
    g.fillRect(0, 0, getWidth(), 30);
    
    const int componentTimeScaleWidth = getWidth();
    const int numDivisions = floor((preSeconds + postSeconds) / resolution);
    
    // draw left-most zero baseline
    g.setFont(font);
    g.setColour(Colour(150, 150, 150));
    g.drawLine(0, 0, 0, getHeight(), 3);
    
    //g.drawText("s:", 0, getHeight()-15, 100, 15, Justification::left, false);
    
    float stepSize = componentTimeScaleWidth / float(numDivisions);

	float zeroMarker = preSeconds / (preSeconds + postSeconds) * componentTimeScaleWidth;
 
    // draw positive values
    float offset = zeroMarker;
    int index = 0;
    
	while (offset < componentTimeScaleWidth)
	{
		int lineEnd = (index % 2 == 0) ? 0 : getHeight() / 2;
        
        g.drawLine(offset, getHeight(), offset, lineEnd, 1);
        g.drawText(String(index * resolution), offset + 6, getHeight()-15, 100, 15, Justification::left, false);

        offset += stepSize;
        index++;
	}

    // draw negative values
    offset = zeroMarker;
    index = 0;

    while (offset > 0)
    {
        int lineEnd = (index % 2 == 0) ? 0 : getHeight() / 2;

        g.drawLine(offset, getHeight(), offset, lineEnd, 1);
        g.drawText(String(-index * resolution), offset + 6, getHeight() - 15, 100, 15, Justification::left, false);

        offset -= stepSize;
        index++;
    }
}

void TimeScale::setWindowSize(float pre, float post)
{
    preSeconds = pre;
    postSeconds = post;

    float windowSize = pre + post;

    if (windowSize > 5)
        resolution = 1.0f;
    else if ((windowSize <= 5) && (windowSize > 0.5))
        resolution = 0.25f;
    else
        resolution = 0.1f;
    
    repaint();
}

ProbeViewerTimeScale::ProbeViewerTimeScale()
{
    rollingViewTimeScale = std::make_unique<TimeScale>();
	averageViewTimeScale = std::make_unique<TimeScale>();

    addAndMakeVisible(rollingViewTimeScale.get());
    addChildComponent(averageViewTimeScale.get());
}


void ProbeViewerTimeScale::resized()
{ 

    if (averageViewTimeScale->isVisible())
    {
        averageViewTimeScale->setBounds(getWidth() - 290, 0, 290, getHeight());
        rollingViewTimeScale->setBounds(margin, 0, getWidth() - margin - 300, getHeight());
    }
    else {
        rollingViewTimeScale->setBounds(margin, 0, getWidth() - margin, getHeight());
    }

}

void ProbeViewerTimeScale::setRollingViewWindowSize(float windowSize)
{
	rollingViewTimeScale->setWindowSize(0, windowSize);
}

void ProbeViewerTimeScale::setAverageViewWindowSize(float preWindow, float postWindow)
{
    averageViewTimeScale->setWindowSize(preWindow, postWindow);
}

void ProbeViewerTimeScale::showAverageView(bool show)
{
	averageViewTimeScale->setVisible(show);
	resized();
}

void ProbeViewerTimeScale::setMarginOffset(float marginOffset)
{
    margin = marginOffset;
    resized();
}
