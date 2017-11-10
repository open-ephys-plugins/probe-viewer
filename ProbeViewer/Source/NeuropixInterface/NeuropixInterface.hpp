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

#ifndef NeuropixInterface_hpp
#define NeuropixInterface_hpp

#include "VisualizerWindowHeaders.h"

namespace ProbeViewer {

enum class ChannelState : int;

class NeuropixInterface : public Component
{
public:
    static const unsigned int MARGIN_WIDTH;

    NeuropixInterface(class ProbeViewerCanvas* canvas);
    virtual ~NeuropixInterface() override;

    void paint(Graphics&) override;

    void mouseMove(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);


    void setNumActiveChannels(int numChannels);
    int getNumActiveChannels() const;


    static const unsigned int MAX_NUM_CHANNELS;
    static const SortedSet<int> refNodes;

    static const unsigned int NUM_PROBE_READ_SITES;
    static const unsigned int PROBE_GRAPHIC_BOTTOM_POS;
    static const int PROBE_VIEW_X_OFFSET;
    static const Path shankPath;
private:

    class ProbeViewerCanvas* canvas;

    int numActiveChannels;

    ScopedPointer<struct ProbeGraphicZoomInfo> zoomInfo;

    Array<ChannelState> channelStatus;
    Array<int> channelSelectionState;

    MouseCursor::StandardCursorType cursorType;

    Rectangle<int> selectionBox;
    bool isSelectionActive = false;

    Colour getChannelColour(uint32 channel);
    int getNearestChannelIdx(int x, int y);
    MouseCursor getMouseCursor();
    void updateProbeSitesRendering();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NeuropixInterface);
};

/**
 *  A data struct for encapsulating the state specific to zooming and
 *  scrolling on the probe interface display.
 */
struct ProbeGraphicZoomInfo
{
    ProbeGraphicZoomInfo() = default;

    bool isMouseOverZoomRegion = {false};
    bool isMouseOverUpperBorder = {false};
    bool isMouseOverLowerBorder = {false};
    bool isMouseOverChannel = {false};

    int zoomHeight = {50};
    int zoomOffset = {0};
    int initialOffset = {0};
    int initialHeight = {0};
    int lowerBound = {513};
    int dragZoneWidth = {10};

    int lowestChan = {0};
    int highestChan = {0};

    float channelHeight = {10};
};

enum class ChannelState : int
{
    reference = -2,
    not_available = -1,
    disabled = 0,
    enabled = 1,
    not_selectable
};

}

#endif /* NeuropixInterface_hpp */
