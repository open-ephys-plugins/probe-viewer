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

#include "VisualizerWindowHeaders.h"
#include "kissfft/kiss_fftr.h"

namespace ProbeViewer {

class ProbeViewerCanvas : public Visualizer
{
public:

    /** Constructor */
    ProbeViewerCanvas(class ProbeViewerNode*);

    /** Destructor */
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

    /** Saves parameters */
    void saveCustomParametersToXml(XmlElement* xml) override;

    /** Loads parameters */
    void loadCustomParametersFromXml(XmlElement* xml) override;

    /**
     *  Overrides from juce::Component
     */

    void paint(Graphics& g) override;
    void resized() override;

    /**
     *  Custom member methods
     */

    void updateChannelBrowsers();

    /**
     *  Get the number of channels that this canvas is currently rendering data
     *  for.
     */
    int getNumChannels();

    /**
     *  Set the height in SCREEN pixels of each channel. This value represents the
     *  height of the channel as it should appear on the screen, as opposed to
     *  the height at which it should be rendered internally.
     */
    void setChannelHeight(float height);

    /**
     *  Get the SCREEN pixel height at which each channel is rendered.
     */
    float getChannelHeight();

    /**
     *  Get the stored sample rate for a specific channel.
     */
    float getChannelSampleRate(int channel);

    /**
     *  Return a pointer for the viewport wrapping the channel canvas area
     */
    class ProbeViewerViewport* getViewportPtr();

    /**
     *  Return a pointer for the canvas component on which the channels are
     *  rendered directly.
     */
    class ChannelViewCanvas* getChannelViewCanvasPtr();
    
    /**
     *  Return a pointer for the Channel Browser component 
     */
    class ChannelBrowser* getChannelBrowserPtr();

    static const float TRANSPORT_WINDOW_TIMEBASE;
#ifdef WIN32
    static const int FFT_ORDER = 8;
    static const int FFT_SIZE = 1 << ProbeViewerCanvas::FFT_ORDER;
    static const int FFT_TARGET_SAMPLE_RATE = 1000;
#else
	static constexpr int FFT_ORDER = 8;
	static constexpr int FFT_SIZE = 1 << ProbeViewerCanvas::FFT_ORDER;
	static constexpr int FFT_TARGET_SAMPLE_RATE = 1000;
#endif

private:
    class ProbeViewerNode* pvProcessor;

    OwnedArray<ChannelBrowser> channelBrowsers;
    std::map<uint16, ChannelBrowser*> channelBrowserMap;

    ScopedPointer<class ChannelViewCanvas> channelsView;
    ScopedPointer<class ProbeViewerTimeScale> timeScale;
    ScopedPointer<class CanvasOptionsBar> optionsBar;
    ScopedPointer<class ProbeViewerViewport> viewport;

    class CircularBuffer* dataBuffer;
    OwnedArray<Array<float>> partialBufferCache;

    std::vector<size_t> inputDownsamplingIndex;
    size_t numSamplesToChunk;


    kiss_fftr_cfg fft_cfg;
    std::vector<float> fftInput;
    kiss_fft_cpx fftOutput[ProbeViewerCanvas::FFT_SIZE/2 + 1];



    class FFTSampleCacheBuffer
    {
    public:

        /**
         *
         */
        FFTSampleCacheBuffer(int size);
        ~FFTSampleCacheBuffer();


        /**
         *  FFTSampleCacheBuffer is non-copyable and non-moveable
         */
        FFTSampleCacheBuffer(const FFTSampleCacheBuffer &) = delete;
        FFTSampleCacheBuffer(FFTSampleCacheBuffer &&) = delete;
        FFTSampleCacheBuffer& operator=(const FFTSampleCacheBuffer &) = delete;
        FFTSampleCacheBuffer& operator=(FFTSampleCacheBuffer &) = delete;


        /**
         *  Resizes the internal memory structure to the size given in the
         *  param.
         *
         *  Calling this method flushes all of the current values stored,
         *  resets the write and read indices, and reverts the structure
         *  to its initialization state at the new buffer size.
         */
        void resize(int size);

        /**
         *  Push one new sample to the end of the buffer.
         *
         *  This method will add a new sample to the buffer, overwriting the
         *  oldest sample and incrementing the write and read indices forward
         *  by one position. The increment automatically wraps.
         */
        void pushSample(const float sample);

        /**
         *  Read one sample from the buffer at the given index.
         *
         *  @param index    The given index must be in the range [0, size), and
         *                  the buffer will automatically adjust for the current
         *                  position of the internal readIdx.
         */
        float readSample(int index) const;


        /**
         *  Return the number of writable samples available to this buffer
         */
        int size() { return bufferSize; }

    private:
        int bufferSize;
        int writeIdx;
        int readIdx;

        std::vector<float> buffer;
    };

    OwnedArray<FFTSampleCacheBuffer> channelFFTSampleBuffer;

    static const std::vector<float> fftWindow;

    int numChannels;
    bool isUpdating;

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
