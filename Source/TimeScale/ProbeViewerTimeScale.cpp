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

ProbeViewerTimeScale::ProbeViewerTimeScale(float timeScale, float resolution)
: timeScale(timeScale)
, resolution(resolution)
, marginWidth(0)
, backgroundGradient(Colour(50,50,50), 0, 0, Colour(25,25,25), 0, 30, false)
, backgroundColour(58, 58, 58)
, foregroundColour(150, 150, 150)
, font("Default", 16, Font::plain)
{
    
}

ProbeViewerTimeScale::~ProbeViewerTimeScale()
{ }

void ProbeViewerTimeScale::paint(Graphics& g)
{
    if (backgroundFillType == GRADIENT)
    {
        g.setGradientFill(backgroundGradient);
    }
    else
    {
        g.setColour(backgroundColour);
    }
    
    g.fillRect(0, 0, getWidth(), 30);
    
    const int componentTimeScaleWidth = getWidth() - marginWidth;
    const int numDivisions = timeScale / resolution;
    
    // draw left-most zero baseline
    g.setFont(font);
    g.setColour(foregroundColour);
    g.drawLine(marginWidth, 0, marginWidth, getHeight(), 3);
    
    g.drawText("s:", marginWidth - 25, 0, 100, getHeight(), Justification::left, false);
    
    float divisionWidth = componentTimeScaleWidth / float(numDivisions);
    for (int division = 1; division <= numDivisions; ++division)
    {
        float xOffset = marginWidth + divisionWidth * division;
        if (division % 4 == 0)
        {
            g.drawLine(xOffset, 0, xOffset, getHeight(), 3);
            g.drawText(String(division * resolution), xOffset + 3, 0, 100, getHeight(), Justification::left, false);
        }
        else if (division % 2 == 0)
        {
            g.drawLine(xOffset, getHeight() / 2, xOffset, getHeight(), 3);
            g.drawText(String(division * resolution), xOffset + 3, 0, 100, getHeight(), Justification::left, false);
        }
        else
        {
            g.drawLine(xOffset, getHeight() / 4 * 3, xOffset, getHeight(), 2);
        }
    }
}

void ProbeViewerTimeScale::resized()
{ }

void ProbeViewerTimeScale::setBackgroundColour(Colour background)
{
    backgroundColour = background;
    backgroundFillType = SOLID;
}

void ProbeViewerTimeScale::setBackgroundColourGradient(ColourGradient background)
{
    backgroundGradient = background;
    backgroundFillType = GRADIENT;
}

void ProbeViewerTimeScale::setForegroundColour(Colour foreground)
{
    
}

void ProbeViewerTimeScale::setMarginOffset(float marginOffset)
{
    marginWidth = marginOffset;
}
