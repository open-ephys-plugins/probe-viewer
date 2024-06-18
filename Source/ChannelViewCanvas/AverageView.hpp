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

#ifndef AverageView_hpp
#define AverageView_hpp

#include "VisualizerWindowHeaders.h"

namespace ProbeViewer
{

class AverageView : public Component,
                    public ComboBox::Listener
{
public:
    /** Constructor */
    AverageView (class ChannelViewCanvas* canvas, class ProbeViewerNode* node);

    /** Destructor */
    virtual ~AverageView() override {}

    /** Updates settings */
    void updateViewSettings();

    /**
    *  Set the height in pixels of the displayed channels (data and reference)
    */
    void setChannelHeight (float height);

    /**
    *  Set the sample rate of the displayed stream
    */
    void setSampleRate (float sampleRate);

    /**
     *  Extracts samples from a circular buffer
     */
    void fillFromBuffer (class CircularBuffer* buffer);

    /** Sets the pre/post time window */
    void setWindow (float preWindow, float postWindow);

    /** Render the view */
    void paint (Graphics& g);

    /** Called when options are changed*/
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    /** Used to set TTL trigger line */
    void mouseDown (const MouseEvent& e);

private:
    AudioBuffer<float> screenBuffer;
    AudioBuffer<float> cacheBuffer;
    Array<int> pixelIndex;
    Array<int> numCachedSamples;

    Image screenBufferImage;

    class ChannelViewCanvas* canvas;
    class ProbeViewerNode* node;

    int numChannels;
    int numTrials;
    float channelHeight;
    float preWindow = 0.5;
    float postWindow = 0.5;
    float samplesPerPixel;
    float sampleRate;
    bool updateImage = false;

    int triggerLine = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AverageView);
};

} // namespace ProbeViewer

#endif /* AverageView_hpp */
