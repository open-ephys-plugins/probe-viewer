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

#include "NeuropixInterface.hpp"

#include "ProbeViewerCanvas.h"

#include "../ChannelViewCanvas/ChannelViewCanvas.hpp"

using namespace ProbeViewer;

NeuropixInterface::NeuropixInterface(ProbeViewerCanvas* canvas_)
: canvas(canvas_)
, cursorType(MouseCursor::NormalCursor)
{
    zoomInfo = new ProbeGraphicZoomInfo;
    zoomInfo->lowerBound = PROBE_GRAPHIC_CHAN1_POS;
    
    
    for (size_t i = 0; i < NUM_PROBE_READ_SITES; ++i)
    {
        ChannelState status;
        
        if (i < MAX_NUM_CHANNELS + 10) // because there are 10 refnodes within 384 active channels
        {
            status = ChannelState::enabled;
        }
        else if (i < 960)
        {
            status = ChannelState::disabled;
        }
        else
        {
            status = ChannelState::not_available;
        }
        
        channelStatus.add(status);
        channelSelectionState.add(0);
    }
    
    for (size_t i = 0; i < refNodes.size(); ++i)
    {
        channelStatus.set(*(refNodes.begin() + i) - 1, ChannelState::reference);
    }
    
    addMouseListener(this, true);
    
    setBufferedToImage(true);
}

NeuropixInterface::~NeuropixInterface()
{ }

void NeuropixInterface::paint(Graphics& g)
{
    const int xOffset = 27;
    
    // draw zoomed out channels
    for (size_t channel = 0; channel < channelStatus.size(); ++channel)
    {
        g.setColour(getChannelColour(channel));
        g.setPixel(xOffset + 3 + ((channel % 2)) * 2, PROBE_GRAPHIC_CHAN1_POS - (channel / 2));
        g.setPixel(xOffset + 3 + ((channel % 2)) * 2 + 1, PROBE_GRAPHIC_CHAN1_POS - (channel / 2));
    }
    
    // draw channel numbers
    g.setColour(Colours::grey);
    g.setFont(12);
    
    int ch = 0;
    for (size_t channel = PROBE_GRAPHIC_CHAN1_POS; channel > 30; channel -= 50)
    {
        g.drawLine(6, channel, 18, channel);
        g.drawLine(44, channel, 54, channel);
        g.drawText(String(ch), 59, int(channel) - 6, 100, 12, Justification::left, false);
        ch += 100;
    }
    
    // draw shank outline
    g.setColour(Colours::lightgrey);
    g.strokePath(shankPath, PathStrokeType(1.0));
    
    // draw zoomed channels
    zoomInfo->lowestChan = (PROBE_GRAPHIC_CHAN1_POS - (zoomInfo->lowerBound - zoomInfo->zoomOffset)) * 2 - 1;
    zoomInfo->highestChan = (PROBE_GRAPHIC_CHAN1_POS - (zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight)) * 2 + 10;
    
    float newChannelHeight = float(getHeight() - 2) / ((zoomInfo->highestChan - zoomInfo->lowestChan) / 2);
    if (zoomInfo->channelHeight != newChannelHeight)
    {
        zoomInfo->channelHeight = newChannelHeight;
        canvas->setChannelHeight(newChannelHeight / 2);
        canvas->resized();
    }
    
    
    for (int channel = zoomInfo->lowestChan; channel <= zoomInfo->highestChan; ++channel)
    {
        if (channel >= 0 && channel < NUM_PROBE_READ_SITES)
        {
            float xLocation = PROBE_VIEW_X_OFFSET - zoomInfo->channelHeight * (1 - (channel % 2));
            float yLocation = getHeight() - zoomInfo->channelHeight - ((channel - zoomInfo->lowestChan - (channel % 2)) / 2 * zoomInfo->channelHeight);
            
            if (channelSelectionState[channel])
            {
                g.setColour(Colours::white);
                g.fillRect(xLocation, yLocation, zoomInfo->channelHeight, zoomInfo->channelHeight);
            }
            
            g.setColour(getChannelColour(channel));
            g.fillRect(xLocation + 1,
                       yLocation + 1,
                       zoomInfo->channelHeight - 2,
                       zoomInfo->channelHeight - 2);
        }
    }
    
    // draw borders around zoom area
    
    g.setColour(Colours::darkgrey.withAlpha(0.7f));
    g.fillRect(25, 0, 15, zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight - 2);
    g.fillRect(25, zoomInfo->lowerBound - zoomInfo->zoomOffset, 15, zoomInfo->zoomOffset + 10);
    
    g.setColour(Colours::darkgrey);
    
    Path upperBorder;
    upperBorder.startNewSubPath(5, zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight - 3);
    upperBorder.lineTo(54, zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight - 3);
    upperBorder.lineTo(100, 1);
    upperBorder.lineTo(200, 1);
    
    Path lowerBorder;
    lowerBorder.startNewSubPath(5, zoomInfo->lowerBound - zoomInfo->zoomOffset);
    lowerBorder.lineTo(54, zoomInfo->lowerBound - zoomInfo->zoomOffset);
    lowerBorder.lineTo(100, getHeight() - 1);
    lowerBorder.lineTo(200, getHeight() - 1);
    
    g.strokePath(upperBorder, PathStrokeType(2.0));
    g.strokePath(lowerBorder, PathStrokeType(2.0));
    
    // draw selection zone
    if (isSelectionActive)
    {
        g.setColour(Colours::white.withAlpha(0.5f));
        g.drawRect(selectionBox);
    }
}

void NeuropixInterface::mouseMove(const MouseEvent &event)
{
    float y = event.y;
    float x = event.x;
    
    //std::cout << x << " " << y << std::endl;
    
    bool isOverZoomRegionNew = false;
    bool isOverUpperBorderNew = false;
    bool isOverLowerBorderNew = false;
    
    if (y > zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight - zoomInfo->dragZoneWidth/2
        && y < zoomInfo->lowerBound - zoomInfo->zoomOffset + zoomInfo->dragZoneWidth/2 &&  x > 9 && x < 54)
    {
        isOverZoomRegionNew = true;
    } else {
        isOverZoomRegionNew = false;
    }
    
    if (isOverZoomRegionNew)
    {
        if (y > zoomInfo->lowerBound - zoomInfo->zoomHeight - zoomInfo->zoomOffset - zoomInfo->dragZoneWidth/2
            && y <  zoomInfo->lowerBound - zoomInfo->zoomHeight - zoomInfo->zoomOffset + zoomInfo->dragZoneWidth/2 )
        {
            isOverUpperBorderNew = true;
            
        } else if (y > zoomInfo->lowerBound  - zoomInfo->zoomOffset - zoomInfo->dragZoneWidth/2
                   && y <  zoomInfo->lowerBound  - zoomInfo->zoomOffset + zoomInfo->dragZoneWidth/2)
        {
            isOverLowerBorderNew = true;
            
        } else {
            isOverUpperBorderNew = false;
            isOverLowerBorderNew = false;
        }
    }
    
    if (isOverZoomRegionNew != zoomInfo->isMouseOverZoomRegion ||
        isOverLowerBorderNew != zoomInfo->isMouseOverLowerBorder ||
        isOverUpperBorderNew != zoomInfo->isMouseOverUpperBorder)
    {
        zoomInfo->isMouseOverZoomRegion = isOverZoomRegionNew;
        zoomInfo->isMouseOverUpperBorder = isOverUpperBorderNew;
        zoomInfo->isMouseOverLowerBorder = isOverLowerBorderNew;
        
        if (!zoomInfo->isMouseOverZoomRegion)
        {
            cursorType = MouseCursor::NormalCursor;
        } else {
            
            if (zoomInfo->isMouseOverUpperBorder)
                cursorType = MouseCursor::TopEdgeResizeCursor;
            else if (zoomInfo->isMouseOverLowerBorder)
                cursorType = MouseCursor::BottomEdgeResizeCursor;
            else
                cursorType = MouseCursor::NormalCursor;
        }
        
        repaint();
    }
    
    if (x > PROBE_VIEW_X_OFFSET - zoomInfo->channelHeight && x < PROBE_VIEW_X_OFFSET + zoomInfo->channelHeight && y < zoomInfo->lowerBound && y > 18)
    {
        zoomInfo->isMouseOverChannel = true;
        
        repaint();
    } else {
        bool isOverChannelNew = false;
        
        if (isOverChannelNew != zoomInfo->isMouseOverChannel)
        {
            zoomInfo->isMouseOverChannel = isOverChannelNew;
            repaint();
        }
    }
}

void NeuropixInterface::mouseDown(const MouseEvent &event)
{
    zoomInfo->initialOffset = zoomInfo->zoomOffset;
    zoomInfo->initialHeight = zoomInfo->zoomHeight;
    
    if (!event.mods.isRightButtonDown())
    {
        if (event.x > PROBE_VIEW_X_OFFSET && event.x < 400)
        {
            for (int i = 0; i < 966; ++i)
            {
                channelSelectionState.set(i, 0);
            }
        }
        
        if (event.x > PROBE_VIEW_X_OFFSET - zoomInfo->channelHeight && event.x < PROBE_VIEW_X_OFFSET + zoomInfo->channelHeight)
        {
            auto chan = getNearestChannelIdx(event.x, event.y);
            
            if (chan >= 0 && chan < 966)
            {
                channelSelectionState.set(chan, 1);
            }
        }
        repaint();
    }
//    else
//    {
//        // Annotation specific stuff
//    }
}

void NeuropixInterface::mouseDrag(const MouseEvent &event)
{
    if (zoomInfo->isMouseOverZoomRegion)
    {
        if (zoomInfo->isMouseOverUpperBorder)
        {
            zoomInfo->zoomHeight = zoomInfo->initialHeight - event.getDistanceFromDragStartY();
            
            if (zoomInfo->zoomHeight > zoomInfo->lowerBound - zoomInfo->zoomOffset - 18)
                zoomInfo->zoomHeight = zoomInfo->lowerBound - zoomInfo->zoomOffset - 18;
        }
        else if (zoomInfo->isMouseOverLowerBorder)
        {
            zoomInfo->zoomOffset = zoomInfo->initialOffset - event.getDistanceFromDragStartY();
        
            if (zoomInfo->zoomOffset < 0)
            {
                zoomInfo->zoomOffset = 0;
            }
            else
            {
                zoomInfo->zoomHeight = zoomInfo->initialHeight + event.getDistanceFromDragStartY();
                if (zoomInfo->zoomHeight < 10)
                    zoomInfo->zoomHeight = 10;
            }
        }
        else
        {
            zoomInfo->zoomOffset = zoomInfo->initialOffset - event.getDistanceFromDragStartY();
            if (zoomInfo->zoomOffset < 0) zoomInfo->zoomOffset = 0;
        }
    }
    else if (event.x > PROBE_VIEW_X_OFFSET && event.x < PROBE_VIEW_X_OFFSET + 100)
    {
        int w = event.getDistanceFromDragStartX();
        int h = event.getDistanceFromDragStartY();
        int x = event.getMouseDownX();
        int y = event.getMouseDownY();
        
        if (w < 0)
        {
            x = x + w; w = -w;
        }
        
        if (h < 0)
        {
            y = y + h; h = -h;
        }
        
        selectionBox = Rectangle<int>(x, y, w, h);
        isSelectionActive = true;
        
        
        int chanStart = getNearestChannelIdx(100, y + h);
        int chanEnd = getNearestChannelIdx(100, y) + 1;
        
        if (x < PROBE_VIEW_X_OFFSET + zoomInfo->channelHeight)
        {
            for (int i = 0; i < 966; i++)
            {
                if (i >= chanStart && i <= chanEnd)
                {
                    if (i % 2 == 1)
                    {
                        if ((x + w > PROBE_VIEW_X_OFFSET) || (x > PROBE_VIEW_X_OFFSET && x < PROBE_VIEW_X_OFFSET + zoomInfo->channelHeight))
                            channelSelectionState.set(i, 1);
                        else
                            channelSelectionState.set(i, 0);
                    } else {
                        if ((x < PROBE_VIEW_X_OFFSET) && (x + w > (PROBE_VIEW_X_OFFSET - zoomInfo->channelHeight)))
                            channelSelectionState.set(i, 1);
                        else
                            channelSelectionState.set(i, 0);
                    }
                } else {
                    if (!event.mods.isShiftDown())
                        channelSelectionState.set(i, 0);
                }
            }
        } else {
            for (int i = 0; i < 966; i++)
            {
                if (!event.mods.isShiftDown())
                    channelSelectionState.set(i, 0);
            }
        }
    }
    
    if (zoomInfo->zoomOffset > (MAX_NUM_CHANNELS / 2) - zoomInfo->zoomHeight)
    {
        zoomInfo->zoomOffset = (MAX_NUM_CHANNELS / 2) - zoomInfo->zoomHeight;
    }
    
    if (zoomInfo->zoomOffset < 0)
    {
        zoomInfo->zoomOffset = 0;
    }
    
    if (zoomInfo->zoomHeight < 10)
        zoomInfo->zoomHeight = 10;
    if (zoomInfo->zoomHeight > 384/2)
        zoomInfo->zoomHeight = 384/2;
    
    // TODO: (kelly) this code is still buggy - the viewport positioning gets wonky when changing the size
    // of the zoom area
    float ratio = 1.0f - (zoomInfo->zoomOffset) / (MAX_NUM_CHANNELS / 2.0f - zoomInfo->zoomHeight);
    auto viewport = canvas->getViewportPtr();
    if (viewport)
    {
        viewport->setViewPositionProportionately(0, ratio);
    }
    
    repaint();
}

void NeuropixInterface::mouseUp(const MouseEvent &event)
{
    if (isSelectionActive)
    {
        isSelectionActive = false;
        repaint();
    }
}

void NeuropixInterface::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails& wheel)
{
    if (event.x > 100 && event.x < 450)
    {
        
        if (wheel.deltaY > 0)
            zoomInfo->zoomOffset += 2;
        else
            zoomInfo->zoomOffset -= 2;
        
        //std::cout << wheel.deltaY << " " << zoomOffset << std::endl;
        
        if (zoomInfo->zoomOffset < 0)
        {
            zoomInfo->zoomOffset = 0;
        } else if (zoomInfo->zoomOffset > (MAX_NUM_CHANNELS / 2) - zoomInfo->zoomHeight)
        {
            zoomInfo->zoomOffset = (MAX_NUM_CHANNELS / 2) - zoomInfo->zoomHeight;
        }
        
        auto ratio = 1.0f - (zoomInfo->zoomOffset) / (MAX_NUM_CHANNELS / 2.0f - zoomInfo->zoomHeight);
        auto viewport = canvas->getViewportPtr();
        viewport->setViewPositionProportionately(0, ratio);
        
        repaint();
    }
}

Colour NeuropixInterface::getChannelColour(uint channel)
{
    switch (channelStatus[channel]) // not available
    {
        case ChannelState::not_available:
            return Colours::grey;
            
        case ChannelState::disabled:
            return Colours::maroon;
            
        case ChannelState::enabled:
            return Colours::yellow;
            
        case ChannelState::reference:
            return Colours::black;
            
        case ChannelState::not_selectable:
        default:
            break;
    }
    
    return Colours::brown;
}

int NeuropixInterface::getNearestChannelIdx(int x, int y)
{
    int chan = ((getHeight() - zoomInfo->channelHeight - y) * 2 / zoomInfo->channelHeight) + zoomInfo->lowestChan + 2;
    
    if (chan % 2 == 1)
        chan += 1;
    
    if (x > 225)
        chan += 1;
    
    return chan;
}

MouseCursor NeuropixInterface::getMouseCursor()
{
    MouseCursor c = MouseCursor(cursorType);
    
    return c;
}








#pragma mark - NeuropixInterface Constants

const unsigned int NeuropixInterface::MARGIN_WIDTH = 30;

const unsigned int NeuropixInterface::NUM_PROBE_READ_SITES = 966;

const unsigned int NeuropixInterface::PROBE_GRAPHIC_CHAN1_POS = 513;

const unsigned int NeuropixInterface::MAX_NUM_CHANNELS = 384;

const int NeuropixInterface::PROBE_VIEW_X_OFFSET = 150;

const SortedSet<int> NeuropixInterface::refNodes = []() -> SortedSet<int> {
    Array<int> r {
        37, 76,
        113, 152,
        189, 228,
        265, 304,
        341, 380,
        421, 460,
        497, 536,
        573, 612,
        649, 688,
        725, 805,
        844, 881,
        920, 957
    };
    
    SortedSet<int> s;
    s.addArray(r.begin(), r.size());
    
    return s;
}();

const Path NeuropixInterface::shankPath = []() -> Path {
    Path p;
    p.startNewSubPath(27, 28);
    p.lineTo(27, PROBE_GRAPHIC_CHAN1_POS + 1);
    p.lineTo(27 + 5, PROBE_GRAPHIC_CHAN1_POS + 9);
    p.lineTo(27 + 10, PROBE_GRAPHIC_CHAN1_POS + 1);
    p.lineTo(27 + 10, 28);
    p.closeSubPath();
    
    return p;
}();
