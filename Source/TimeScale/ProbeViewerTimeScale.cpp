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

#include "../ChannelViewCanvas/ChannelViewCanvas.hpp"

using namespace ProbeViewer;

TimeScale::TimeScale(ChannelViewCanvas* canvas_, bool isForAverageView_) :
	canvas(canvas_),
	isForAverageView(isForAverageView_),
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

    if (windowSize > 4)
        resolution = 1.0f;
    else if ((windowSize <= 4) && (windowSize > 0.5))
        resolution = 0.25f;
    else if ((windowSize <= 0.5) && (windowSize > 0.3))
        resolution = 0.1f;
    else
        resolution = 0.05f;

    if (isForAverageView)
        canvas->updateAverageViewWindow(preSeconds, postSeconds);
    else
        canvas->updateRollingViewWindow(postSeconds);
    
    repaint();
}


void TimeScale::mouseDown(const MouseEvent& e)
{

    if (e.mods.isRightButtonDown())
    {
        PopupMenu m;

        if (isForAverageView)
        {
            m.addItem(98, "Pre Window", false);
            m.addItem(2, "50 ms", true, preSeconds == 0.05f);
            m.addItem(3, "0.1 s", true, preSeconds == 0.1f);
            m.addItem(4, "0.2 s", true, preSeconds == 0.2f);
            m.addItem(5, "0.5 s", true, preSeconds == 0.5f);
            m.addItem(6, "1.0 s", true, preSeconds == 1.0f);
            m.addSeparator();
			m.addItem(99, "Post Window", false);
			m.addItem(7, "50 ms", true, postSeconds == 0.05f);
			m.addItem(8, "0.1 s", true, postSeconds == 0.1f);
			m.addItem(9, "0.2 s", true, postSeconds == 0.2f);
			m.addItem(10, "0.5 s", true, postSeconds == 0.5f);
			m.addItem(11, "1.0 s", true, postSeconds == 1.0f);
            
            const int result = m.show();

            switch (result)
            {
            case 0: // nothing selected
                return;
            case 2:
                preSeconds = 0.05f;
                break;
			case 3:
				preSeconds = 0.1f;
				break;
			case 4:
				preSeconds = 0.2f;
				break;
			case 5:
				preSeconds = 0.5f;
				break;
			case 6:
				preSeconds = 1.0f;
				break;
            case 7:
				postSeconds = 0.05f;
				break;
			case 8:
				postSeconds = 0.1f;
				break;
			case 9:
				postSeconds = 0.2f;
				break;
			case 10:
				postSeconds = 0.5f;
				break;
			case 11:
				postSeconds = 1.0f;
				break;
            }
        }
        else
        {
            m.addItem(98, "Window Size", false);
            m.addItem(2, "5 s", true, postSeconds == 5.f);
            m.addItem(3, "8 s", true, postSeconds == 8.f);
            m.addItem(4, "10 s", true, postSeconds == 10.f);
            m.addItem(5, "20 s", true, postSeconds == 20.f);

            const int result = m.show();

            switch (result)
            {
            case 0: // nothing selected
                return;
            case 2:
                postSeconds = 5.f;
                break;
            case 3:
                postSeconds = 8.f;
                break;
            case 4:
                postSeconds = 10.f;
                break;
            case 5:
                postSeconds = 20.f;
                break;
            }
        }

        setWindowSize(preSeconds, postSeconds);
    }

}

ProbeViewerTimeScale::ProbeViewerTimeScale(class ChannelViewCanvas* canvas)
{
    rollingViewTimeScale = std::make_unique<TimeScale>(canvas, false);
	averageViewTimeScale = std::make_unique<TimeScale>(canvas, true);

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

void ProbeViewerTimeScale::getWindowSize(float* rollingWindowSize, float* averageWindowPre, float* averageWindowPost)
{
	*rollingWindowSize = rollingViewTimeScale->postSeconds;
	*averageWindowPre = averageViewTimeScale->preSeconds;
	*averageWindowPost = averageViewTimeScale->postSeconds;
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
