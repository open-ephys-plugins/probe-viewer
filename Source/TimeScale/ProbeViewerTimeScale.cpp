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
, font("Default", 16, Font::plain)
{
    
}

ProbeViewerTimeScale::~ProbeViewerTimeScale()
{ }

void ProbeViewerTimeScale::paint(Graphics& g)
{
    g.setColour(Colour(35, 35, 35));
    
    g.fillRect(0, 0, getWidth(), 30);
    
    const int componentTimeScaleWidth = getWidth() - marginWidth;
    const int numDivisions = timeScale / resolution;
    
    // draw left-most zero baseline
    g.setFont(font);
    g.setColour(Colour(150, 150, 150));
    g.drawLine(marginWidth, 0, marginWidth, getHeight(), 3);
    
    g.drawText("s:", marginWidth - 25, getHeight()-15, 100, 15, Justification::left, false);
    
    float divisionWidth = componentTimeScaleWidth / float(numDivisions);
    for (int division = 1; division <= numDivisions; ++division)
    {
        float xOffset = marginWidth + divisionWidth * division;
        if (division % 4 == 0)
        {
            g.drawLine(xOffset, 0, xOffset, getHeight(), 3);
            g.drawText(String(division * resolution), xOffset + 6, getHeight()-15, 100, 15, Justification::left, false);
        }
        else if (division % 2 == 0)
        {
            g.drawLine(xOffset, getHeight() / 2, xOffset, getHeight(), 3);
            g.drawText(String(division * resolution), xOffset + 6, getHeight()-15, 100, 15, Justification::left, false);
        }
        else
        {
            g.drawLine(xOffset, getHeight() / 4 * 3, xOffset, getHeight(), 2);
        }
    }
}

void ProbeViewerTimeScale::resized()
{ }

void ProbeViewerTimeScale::setMarginOffset(float marginOffset)
{
    marginWidth = marginOffset;
}
