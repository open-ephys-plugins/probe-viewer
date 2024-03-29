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

class ChannelBrowser : public Component, public Timer
{
public:
    static const unsigned int MARGIN_WIDTH;

    /** Constructor */
    ChannelBrowser(class ProbeViewerCanvas* canvas, int id);

    /** Destructor*/
    virtual ~ChannelBrowser() override { }

    /** Renders the channel browser*/
    void paint(Graphics&) override;

    /** Returns the height (in pixels) of each channel */
    float getChannelHeight();

    /** Timer callback*/
    void timerCallback() override { repaint(); stopTimer(); }

    /** Mouse callbacks */
    void mouseMove(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel);

    /** Add a new channel to draw*/
    void addChannel(int channelNum, String channelName, float depth, int electrode_index);  

    /** Creates colors after all channels have been added */
    void createChannelColours();

    /** Called before channels have been added*/
    void reset();

    /** Called after all channels have been added*/
    void updateChannelSitesRendering();

    /** Returns the total number of channels drawn*/
    int getNumChannels() const;

    /** Set region info (e.g. from Pinpoint or Trajectory Explorer) */
    void setRegions(Array<int>& electrodeInds, Array<String>& regionNames, Array<Colour>& regionColours);

    float getViewportScrollPositionRatio();

    void saveParameters(XmlElement* xml);

    void loadParameters(XmlElement* xml);

    static const unsigned int NUM_PROBE_READ_SITES;
    static const int PROBE_VIEW_X_OFFSET;

    int id;

private:

    class ProbeViewerCanvas* canvas;

    Array<Colour> customColours;
    bool regionInfoIsAvailable = false;

    int numChannels;
    int graphicBottomPos;

    ScopedPointer<struct ProbeGraphicZoomInfo> zoomInfo;

    MouseCursor::StandardCursorType cursorType;

    Array<String> regionNames;

    Path shankPath;

    Rectangle<int> selectionBox;
    bool isSelectionActive = false;
    
    bool isMouseActionLocked = false;

    struct ChannelMetadata{
        int num = -1;
        String name = "";
        float depth = -1;
        int electrode_index = -1;
        Colour colour;
    };

    Array<ChannelMetadata> channelMetadata;
    Array<int> channelOrder;

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
