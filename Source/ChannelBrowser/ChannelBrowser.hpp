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

#ifndef ChannelBrowser_hpp
#define ChannelBrowser_hpp

#include "VisualizerWindowHeaders.h"

namespace ProbeViewer {

class ChannelBrowser : public Component
{
public:
    static const unsigned int MARGIN_WIDTH;

    ChannelBrowser(class ProbeViewerCanvas* canvas, int id);
    virtual ~ChannelBrowser() override;

    void paint(Graphics&) override;

    float getChannelHeight();

    void mouseMove(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);

    void addChannel(int channelNum, String channelName);    
    int getNumChannels() const;

    void reset();

    void updateChannelSitesRendering();
    
    float getViewportScrollPositionRatio();

    void saveParameters(XmlElement* xml);

    void loadParameters(XmlElement* xml);

    static const unsigned int NUM_PROBE_READ_SITES;
    static const int PROBE_VIEW_X_OFFSET;

    int id;

private:

    class ProbeViewerCanvas* canvas;

    int numChannels;
    int graphicBottomPos;

    ScopedPointer<struct ProbeGraphicZoomInfo> zoomInfo;

    MouseCursor::StandardCursorType cursorType;

    Path shankPath;

    Rectangle<int> selectionBox;
    bool isSelectionActive = false;
    
    bool isMouseActionLocked = false;

    struct ChannelMetadata{
        int num = -1;
        String name = "";
    };

    Array<ChannelMetadata> channelMetadata;

    Colour getChannelColour(int channel);
    int getNearestChannelIdx(int x, int y);
    MouseCursor getMouseCursor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelBrowser);
};

/**
 *  A data struct for encapsulating the state specific to zooming and
 *  scrolling on the Channel Browser display.
 */
struct ProbeGraphicZoomInfo
{
    ProbeGraphicZoomInfo() = default;

    bool isMouseOverZoomRegion = {false};
    bool isMouseOverUpperBorder = {false};
    bool isMouseOverLowerBorder = {false};
    bool isMouseOverChannel = {false};

    int zoomHeight = {16};
    int zoomOffset = {0};
    int initialOffset = {0};
    int initialHeight = {0};
    int lowerBound = {513};
    int dragZoneWidth = {8};

    int lowestChan = {0};
    int highestChan = {0};

    float channelHeight = {10};
    
    juce::Point<int> lastPosition = {0, 0};
    float viewportScrollPositionRatio = {1};
};

}

#endif /* ChannelBrowser_hpp */
