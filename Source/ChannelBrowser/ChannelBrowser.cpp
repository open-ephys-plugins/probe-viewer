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

#include "ChannelBrowser.hpp"

#include "../ProbeViewerCanvas.h"

#include "../ChannelViewCanvas/ChannelViewCanvas.hpp"

using namespace ProbeViewer;

ChannelBrowser::ChannelBrowser(ProbeViewerCanvas* canvas_, int id_)
: canvas(canvas_)
, id(id_)
, cursorType(MouseCursor::NormalCursor)
, numChannels(0)
, graphicBottomPos(0)
{
    zoomInfo = new ProbeGraphicZoomInfo;
    
    setBufferedToImage(true);
}

float ChannelBrowser::getChannelHeight()
{
    return zoomInfo->channelHeight;
}

void ChannelBrowser::paint(Graphics& g)
{
    if(numChannels <= 0)
        return;
    
    const int xOffset = 27;
    
    // draw zoomed out channels
    for (int channel = 0; channel < numChannels; ++channel)
    {
        g.setColour(getChannelColour(channelOrder[channel]));
        g.fillRect(xOffset + 3, graphicBottomPos - channel, 4, 1);
        // g.fillRect(xOffset + 3 + ((channel % 2)) * 2 + 1, ChannelBrowser::PROBE_GRAPHIC_BOTTOM_POS - (channel / 2), 1, 1);
    }
    
    // draw channel numbers
    g.setColour(Colours::grey);
    g.setFont(12);
    
    int ch = 0;
    for (int channel = graphicBottomPos; channel > 10; channel -= 50)
    {
        g.drawLine(10, channel, 18, channel);
        g.drawLine(44, channel, 52, channel);
        int actual_channel = channelOrder[ch];
        g.drawText(channelMetadata[actual_channel].name, 56, int(channel) - 6, 100, 12, Justification::left, false);

        ch == 0 ? ch += 49 : ch += 50;
    }
    
    // draw shank outline
    g.setColour(Colours::lightgrey);
    g.drawRect(xOffset, 8, 10, graphicBottomPos - 2);
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
    
    Font chanFont = Font("Fira Code", "Bold", 12.0f);
    g.setFont(chanFont);

    // LOGC("*********** LOW CHANNEL: ", zoomInfo->lowestChan, " HIGH CHANNEL: ", zoomInfo->highestChan);
    for (int channel = zoomInfo->lowestChan; channel <= zoomInfo->highestChan; ++channel)
    {
        if (channel >= 0 && channel < numChannels)
        {

            float iconHeight = zoomInfo->channelHeight;

            float xLocation = PROBE_VIEW_X_OFFSET - (iconHeight / 2);
            float yLocation = getHeight() - iconHeight - ((channel - zoomInfo->lowestChan) * zoomInfo->channelHeight);

            g.setColour(getChannelColour(channelOrder[channel]));

            g.fillRect(xLocation + 3, yLocation, float(PROBE_VIEW_X_OFFSET) - 3.0f, iconHeight + 1.0f);

            float alpha = 0.0f;

            if (zoomInfo->zoomHeight < 10)
            {
                alpha = 1.0f;
            }
            else if (zoomInfo->zoomHeight >= 10 && zoomInfo->zoomHeight < 30)
            {
                if (channel == 0 || (channel + 1) % 10 == 0)
                    alpha = 1.0f;
                else 
                {
                    alpha = float(20 - (zoomInfo->zoomHeight - 10)) / 20.0f;
                }
           
            }
            else if (zoomInfo->zoomHeight >= 30 && zoomInfo->zoomHeight < 250)
            {
                if (channel == 0 || (channel + 1) % 50 == 0)
                    alpha = 1.0f;
                else
                {
                    if ((channel + 1) % 10 == 0)
                        alpha = float(250 - zoomInfo->zoomHeight) / 250.0f;
                }
            }
            else {
                if (channel == 0 || (channel + 1) % 50 == 0)
                    alpha = 1.0f;
            }
            
            if (alpha > 0.0f)
            {
                g.setColour(Colours::lightgrey.withAlpha(alpha));
                int actual_channel = channelOrder[channel];
                int depth = int(channelMetadata[actual_channel].depth);

                String nameText = channelMetadata[actual_channel].name;
                
                float stringWidth = chanFont.getStringWidth(nameText);

                g.drawLine(xLocation - 5, yLocation + 6.5, xLocation, yLocation + 6.5, 2.0f);
                
                g.drawText(nameText,
                    xLocation - stringWidth - 7,
                    yLocation + 2,
                    stringWidth,
                    12,
                    Justification::centredRight);


                if (depth > 0)
                {
                    if (depth % 2 == 1)
                        depth -= 1;

                    String depthText = String(int(depth));

                    stringWidth = chanFont.getStringWidth(depthText);

                    g.setColour(Colours::grey.withAlpha(alpha));
                    g.drawText(depthText,
                        xLocation - stringWidth - 7,
                        yLocation + 15,
                        stringWidth,
                        12,
                        Justification::centredRight);

                }
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

void ChannelBrowser::mouseMove(const MouseEvent &event)
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

void ChannelBrowser::mouseDown(const MouseEvent &event)
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

void ChannelBrowser::mouseDrag(const MouseEvent &event)
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
    
    if (zoomInfo->zoomOffset > numChannels - zoomInfo->zoomHeight)
    {
        zoomInfo->zoomOffset = numChannels - zoomInfo->zoomHeight;
    }
    
    if (zoomInfo->zoomOffset < 0)
    {
        zoomInfo->zoomOffset = 0;
    }
    
    if (zoomInfo->zoomHeight < 10)
        zoomInfo->zoomHeight = 10;

    int maxZoomHeight = numChannels > 384 ? 384 : numChannels;
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
    const float viewportHeight = (numChannels) - zoomInfo->zoomHeight;
    const float zoomAreaTopEdge = viewportHeight - zoomInfo->zoomOffset;
    zoomInfo->viewportScrollPositionRatio = zoomAreaTopEdge / viewportHeight;
    
    auto viewport = canvas->getViewportPtr();
    if (viewport)
    {
        viewport->setViewPositionProportionately(0, zoomInfo->viewportScrollPositionRatio);
    }
    
    repaint();
}

void ChannelBrowser::mouseUp(const MouseEvent &event)
{
    isMouseActionLocked = false;
    
    if (isSelectionActive)
    {
        isSelectionActive = false;
        repaint();
    }
}

void ChannelBrowser::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails& wheel)
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
        } else if (zoomInfo->zoomOffset > (numChannels - zoomInfo->zoomHeight))
        {
            zoomInfo->zoomOffset = numChannels - zoomInfo->zoomHeight;
        }
        
        // update the viewport
        const float viewportHeight = numChannels - zoomInfo->zoomHeight;
        const float zoomAreaTopEdge = viewportHeight - zoomInfo->zoomOffset;
        zoomInfo->viewportScrollPositionRatio = zoomAreaTopEdge / viewportHeight;
        auto viewport = canvas->getViewportPtr();
        viewport->setViewPositionProportionately(0, zoomInfo->viewportScrollPositionRatio);
        
        repaint();
    }
}

void ChannelBrowser::addChannel(int chanNum, String chanName, float depth)
{
    ChannelMetadata chanData = ChannelMetadata();
    chanData.num = chanNum;
    chanData.name = chanName;
    chanData.depth = depth;

    channelMetadata.add(chanData);
    numChannels++;
}

int ChannelBrowser::getNumChannels() const
{
    return numChannels;
}

void ChannelBrowser::reset()
{
    numChannels = 0;
    channelMetadata.clear();
    channelOrder.clear();
}

float ChannelBrowser::getViewportScrollPositionRatio()
{
    return zoomInfo->viewportScrollPositionRatio;
}

void ChannelBrowser::saveParameters(XmlElement* xml)
{
    XmlElement* xmlNode = xml->createNewChildElement("STREAM");

    xmlNode->setAttribute("id", id);
    xmlNode->setAttribute("zoomLowerBound", zoomInfo->lowerBound);
    xmlNode->setAttribute("zoomOffset", zoomInfo->zoomOffset);
    xmlNode->setAttribute("zoomHeight", zoomInfo->zoomHeight);
}

void ChannelBrowser::loadParameters(XmlElement* xml)
{
    zoomInfo->lowerBound =  xml->getIntAttribute("zoomLowerBound", graphicBottomPos + 10);
    zoomInfo->zoomOffset =  xml->getIntAttribute("zoomOffset", 0);
    zoomInfo->zoomHeight = xml->getIntAttribute("zoomHeight", numChannels > 128 ? 50 : 16);

    const float viewportHeight = numChannels - zoomInfo->zoomHeight;
    const float zoomAreaTopEdge = viewportHeight - zoomInfo->zoomOffset;
    zoomInfo->viewportScrollPositionRatio = zoomAreaTopEdge / viewportHeight;
    auto viewport = canvas->getViewportPtr();
    viewport->setViewPositionProportionately(0, zoomInfo->viewportScrollPositionRatio);

    repaint();
}

Colour ChannelBrowser::getChannelColour(int channel)
{
    return Colours::yellow.interpolatedWith(Colours::purple, (float)channel/(float)numChannels);
}

int ChannelBrowser::getNearestChannelIdx(int x, int y)
{
    int chan = ((getHeight() - zoomInfo->channelHeight - y) / zoomInfo->channelHeight) + zoomInfo->lowestChan + 1;
    
    return chan;
}

MouseCursor ChannelBrowser::getMouseCursor()
{
    MouseCursor c = MouseCursor(cursorType);
    
    return c;
}

void ChannelBrowser::updateChannelSitesRendering()
{
    graphicBottomPos = numChannels + 10;
    zoomInfo->lowerBound = graphicBottomPos;
    zoomInfo->zoomHeight = numChannels > 128 ? 50 : 16;

    channelOrder.clear();

    if (numChannels > 0)
    {

        std::vector<float> depths(numChannels);

        bool allSame = true;
        float last = channelMetadata[0].depth;

        for (int i = 0; i < numChannels; i++)
        {
            float depth = channelMetadata[i].depth;

            if (depth != last)
                allSame = false;

            depths[i] = depth;

            last = depth;
        }

        if (allSame)
        {
            //LOGC("No depth info found.");
            for (int i = 0; i < numChannels; i++)
                channelOrder.add(i);
        }
        else {
            //LOGC("Sorting channels by depth.");
            std::vector<int> V(numChannels);

            std::iota(V.begin(), V.end(), 0); //Initializing
            sort(V.begin(), V.end(), [&](int i, int j) {return depths[i] <= depths[j]; });

            for (int i = 0; i < numChannels; i++)
            {
                // re-order by depth
                channelOrder.add(V[i]);
            }
        }

    }

    repaint();
}


#pragma mark - ChannelBrowser Constants

const unsigned int ChannelBrowser::MARGIN_WIDTH = 30;

const int ChannelBrowser::PROBE_VIEW_X_OFFSET = 155;
