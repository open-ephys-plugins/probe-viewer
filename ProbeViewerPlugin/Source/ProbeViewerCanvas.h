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

#ifndef __PROBEVIEWERCANVAS_H__
#define __PROBEVIEWERCANVAS_H__

#include <VisualizerWindowHeaders.h>

namespace ProbeViewer {
    
class ProbeViewerCanvas : public Visualizer
{
public:
    ProbeViewerCanvas(class ProbeViewerNode*);
    
    virtual ~ProbeViewerCanvas() override;
    
    
    
    /**
     *  Required overrides from Visualizer
     */
    
    /** Called when the component's tab becomes visible again.*/
    virtual void refreshState() override;
    
    /** Called when parameters of underlying data processor are changed.*/
    virtual void update() override;
    
    /** Called instead of "repaint" to avoid redrawing underlying components if not necessary.*/
    virtual void refresh() override;
    
    /** Called when data acquisition is active.*/
    virtual void beginAnimation() override;
    
    /** Called when data acquisition ends.*/
    virtual void endAnimation() override;
    
    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, float) override {}               // unused
    
    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, int, int, float) override {}     // unused
    
    
    
    /**
     *  Overrides from juce::Component
     */
    
    void paint(Graphics& g) override;
    void resized() override;
    
    
    
    /**
     *  Custom member methods
     */
    
    void setNumChannels(int numChannels);
    int getNumChannels();
    
    void setChannelHeight(float height);
    float getChannelHeight();
    
    float getChannelSampleRate(int channel);
    
    /** Delegates a subprocessor index for drawing to the LfpDisplay referenced by this
     this canvas */
    void setDrawableSubprocessor(int idx);
    
    class ProbeViewerViewport* getViewportPtr();
    class ChannelViewCanvas* getChannelViewCanvasPtr();
    
    static const float TRANSPORT_WINDOW_TIMEBASE;
    
private:
    class ProbeViewerNode* pvProcessor;
    ScopedPointer<class NeuropixInterface> interface;
    ScopedPointer<class ChannelViewCanvas> channelsView;
    ScopedPointer<class ProbeViewerViewport> viewport;
    
    class CircularBuffer* dataBuffer;
    OwnedArray<Array<float>> partialBufferCache;
    
    int numChannels;
    
    void updateScreenBuffers();
    int getNumCachedSamples(int channel);
    float popFrontCachedSampleForChannel(int channel);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeViewerCanvas);
};
    
class ProbeViewerViewport : public Viewport
{
public:
    ProbeViewerViewport(ProbeViewerCanvas*, class ChannelViewCanvas*);
    virtual ~ProbeViewerViewport() override;
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);
    
private:
    ProbeViewerCanvas* canvas;
    class ChannelViewCanvas* channelsView;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeViewerViewport);
};
    
}

#endif /* __PROBEVIEWERCANVAS_H__ */
