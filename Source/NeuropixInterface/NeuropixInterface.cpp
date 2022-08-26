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

#include "../ProbeViewerCanvas.h"

#include "../ChannelViewCanvas/ChannelViewCanvas.hpp"

using namespace ProbeViewer;

NeuropixInterface::NeuropixInterface(ProbeViewerCanvas* canvas_)
: canvas(canvas_)
, cursorType(MouseCursor::NormalCursor)
, numActiveChannels(0)
, graphicBottomPos(0)
{
    zoomInfo = new ProbeGraphicZoomInfo;
    // zoomInfo->lowerBound = NeuropixInterface::PROBE_GRAPHIC_BOTTOM_POS;
    
    
    // for (int i = 0; i < refNodes.size(); ++i)
    // {
    //     channelStatus.set(*(refNodes.begin() + i) - 1, ChannelState::reference);
    // }
    
    // addMouseListener(this, true);
    
    setBufferedToImage(true);
}

NeuropixInterface::~NeuropixInterface()
{ }

void NeuropixInterface::paint(Graphics& g)
{
    if(numActiveChannels <= 0)
        return;
    
    const int xOffset = 27;
    
    // draw zoomed out channels
    for (int channel = 0; channel < numActiveChannels; ++channel)
    {
        g.setColour(getChannelColour(channel));
        g.fillRect(xOffset + 3, graphicBottomPos - channel, 4, 1);
        // g.fillRect(xOffset + 3 + ((channel % 2)) * 2 + 1, NeuropixInterface::PROBE_GRAPHIC_BOTTOM_POS - (channel / 2), 1, 1);
    }
    
    // draw channel numbers
    g.setColour(Colours::grey);
    g.setFont(12);
    
    int ch = 0;
    for (int channel = graphicBottomPos; channel > 30; channel -= 50)
    {
        g.drawLine(6, channel, 18, channel);
        g.drawLine(44, channel, 54, channel);
        g.drawText(String(ch), 59, int(channel) - 6, 100, 12, Justification::left, false);
        ch += 50;
    }
    
    // draw shank outline
    g.setColour(Colours::lightgrey);
    g.drawRect(xOffset, 8, 10, graphicBottomPos - 4);
    // g.strokePath(shankPath, PathStrokeType(1.0));
    
    // draw zoomed channels
    zoomInfo->lowestChan = graphicBottomPos - (zoomInfo->lowerBound - zoomInfo->zoomOffset);
    zoomInfo->highestChan = (graphicBottomPos - (zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight));
    
    float newChannelHeight = float(getHeight() - 2) / ((zoomInfo->highestChan - zoomInfo->lowestChan));
    if (zoomInfo->channelHeight != newChannelHeight)
    {
        zoomInfo->channelHeight = newChannelHeight;
        canvas->setChannelHeight(newChannelHeight);
        canvas->resized();
    }
    
    // LOGC("*********** LOW CHANNEL: ", zoomInfo->lowestChan, " HIGH CHANNEL: ", zoomInfo->highestChan);
    for (int channel = zoomInfo->lowestChan; channel <= zoomInfo->highestChan; ++channel)
    {
        if (channel >= 0 && channel < numActiveChannels)
        {
            float xLocation = PROBE_VIEW_X_OFFSET - (zoomInfo->channelHeight / 2);
            float yLocation = getHeight() - zoomInfo->channelHeight - ((channel - zoomInfo->lowestChan) * zoomInfo->channelHeight);
            
            g.setColour(Colours::black);
            g.drawEllipse(xLocation, yLocation, zoomInfo->channelHeight, zoomInfo->channelHeight, 1.0f);
            
            g.setColour(getChannelColour(channel));
            g.fillEllipse(xLocation + 1,
                       yLocation + 1,
                       zoomInfo->channelHeight - 2,
                       zoomInfo->channelHeight - 2);
            
            if(zoomInfo->channelHeight > 26.0f)
            {
                g.setColour(Colours::black);
                g.setFont(Font("Silkscreen", "Regular", 10.0f));
                g.drawText(String(channel),
                    xLocation + 2,
                    yLocation + 2,
                    zoomInfo->channelHeight - 4,
                    zoomInfo->channelHeight - 4,
                    Justification::centred);
            }
        }
    }
    
    // draw borders around zoom area
    
    g.setColour(Colours::darkgrey.withAlpha(0.7f));
    g.fillRect(25, 0, 15, zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight);
    g.fillRect(25, zoomInfo->lowerBound - zoomInfo->zoomOffset, 15, zoomInfo->zoomOffset + 10);
    
    g.setColour(Colours::grey);
    
    Path upperBorder;
    upperBorder.startNewSubPath(5, zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight);
    upperBorder.lineTo(54, zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight);
    upperBorder.lineTo(100, 1);
    upperBorder.lineTo(200, 1);
    
    Path lowerBorder;
    lowerBorder.startNewSubPath(5, zoomInfo->lowerBound - zoomInfo->zoomOffset);
    lowerBorder.lineTo(54, zoomInfo->lowerBound - zoomInfo->zoomOffset);
    lowerBorder.lineTo(100, getHeight() - 1);
    lowerBorder.lineTo(200, getHeight() - 1);
    
    g.strokePath(upperBorder, PathStrokeType(2.0));
    g.strokePath(lowerBorder, PathStrokeType(2.0));

}

void NeuropixInterface::mouseMove(const MouseEvent &event)
{
    if (isMouseActionLocked) return;
    
    float y = event.y;
    float x = event.x;
    
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
    zoomInfo->lastPosition = event.getOffsetFromDragStart();
    
    if (!event.mods.isRightButtonDown())
    {
        if (zoomInfo->isMouseOverZoomRegion
            || zoomInfo->isMouseOverUpperBorder
            || zoomInfo->isMouseOverLowerBorder)
        {
            isMouseActionLocked = true;
        }
        
        repaint();
    }
}

void NeuropixInterface::mouseDrag(const MouseEvent &event)
{
    if (event.getOffsetFromDragStart() == zoomInfo->lastPosition) return;
    zoomInfo->lastPosition = event.getOffsetFromDragStart();
    
    if (zoomInfo->isMouseOverZoomRegion)
    {
        if (zoomInfo->isMouseOverUpperBorder)
        {
            zoomInfo->zoomHeight = zoomInfo->initialHeight - event.getDistanceFromDragStartY();
            
            if (zoomInfo->zoomHeight > zoomInfo->lowerBound - zoomInfo->zoomOffset)
                zoomInfo->zoomHeight = zoomInfo->lowerBound - zoomInfo->zoomOffset;
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
    
    if (zoomInfo->zoomOffset > numActiveChannels - zoomInfo->zoomHeight)
    {
        zoomInfo->zoomOffset = numActiveChannels - zoomInfo->zoomHeight;
    }
    
    if (zoomInfo->zoomOffset < 0)
    {
        zoomInfo->zoomOffset = 0;
    }
    
    if (zoomInfo->zoomHeight < 10)
        zoomInfo->zoomHeight = 10;

    int maxZoomHeight = numActiveChannels > 64 ? (numActiveChannels/2) : numActiveChannels;
    if (zoomInfo->zoomHeight > maxZoomHeight)
        zoomInfo->zoomHeight = maxZoomHeight;
    
    // draw zoomed channels
    zoomInfo->lowestChan = graphicBottomPos - (zoomInfo->lowerBound - zoomInfo->zoomOffset);
    zoomInfo->highestChan = (graphicBottomPos - (zoomInfo->lowerBound - zoomInfo->zoomOffset - zoomInfo->zoomHeight));
    
    float newChannelHeight = float(getHeight() - 2) / (zoomInfo->highestChan - zoomInfo->lowestChan);
    if (zoomInfo->channelHeight != newChannelHeight)
    {
        zoomInfo->channelHeight = newChannelHeight;
        canvas->setChannelHeight(newChannelHeight);
        canvas->resized();
    }
    
    // update the viewport
    const float viewportHeight = (numActiveChannels) - zoomInfo->zoomHeight;
    const float zoomAreaTopEdge = viewportHeight - zoomInfo->zoomOffset;
    zoomInfo->viewportScrollPositionRatio = zoomAreaTopEdge / viewportHeight;
    
    auto viewport = canvas->getViewportPtr();
    if (viewport)
    {
        viewport->setViewPositionProportionately(0, zoomInfo->viewportScrollPositionRatio);
    }
    
    repaint();
}

void NeuropixInterface::mouseUp(const MouseEvent &event)
{
    isMouseActionLocked = false;
    
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
        
        
        if (zoomInfo->zoomOffset < 0)
        {
            zoomInfo->zoomOffset = 0;
        } else if (zoomInfo->zoomOffset > (numActiveChannels - zoomInfo->zoomHeight))
        {
            zoomInfo->zoomOffset = numActiveChannels - zoomInfo->zoomHeight;
        }
        
        // update the viewport
        const float viewportHeight = numActiveChannels - zoomInfo->zoomHeight;
        const float zoomAreaTopEdge = viewportHeight - zoomInfo->zoomOffset;
        zoomInfo->viewportScrollPositionRatio = zoomAreaTopEdge / viewportHeight;
        auto viewport = canvas->getViewportPtr();
        viewport->setViewPositionProportionately(0, zoomInfo->viewportScrollPositionRatio);
        
        repaint();
    }
}

void NeuropixInterface::setNumActiveChannels(int numChannels)
{
    numActiveChannels = numChannels;
    
    updateProbeSitesRendering();
}

int NeuropixInterface::getNumActiveChannels() const
{
    return numActiveChannels;
}

float NeuropixInterface::getViewportScrollPositionRatio()
{
    return zoomInfo->viewportScrollPositionRatio;
}

Colour NeuropixInterface::getChannelColour(int channel)
{
    return Colours::yellow.interpolatedWith(Colours::purple, (float)channel/(float)numActiveChannels);
}

int NeuropixInterface::getNearestChannelIdx(int x, int y)
{
    int chan = ((getHeight() - zoomInfo->channelHeight - y) / zoomInfo->channelHeight) + zoomInfo->lowestChan + 1;
    
    return chan;
}

MouseCursor NeuropixInterface::getMouseCursor()
{
    MouseCursor c = MouseCursor(cursorType);
    
    return c;
}

void NeuropixInterface::updateProbeSitesRendering()
{
    graphicBottomPos = numActiveChannels + 10;
    zoomInfo->lowerBound = graphicBottomPos;
    zoomInfo->zoomHeight = numActiveChannels > 128 ? 50 : 16;
    repaint();
}


#pragma mark - NeuropixInterface Constants

const unsigned int NeuropixInterface::MARGIN_WIDTH = 30;

const int NeuropixInterface::PROBE_VIEW_X_OFFSET = 150;
