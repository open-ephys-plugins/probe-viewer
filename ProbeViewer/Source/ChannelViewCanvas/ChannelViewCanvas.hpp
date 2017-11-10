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

#ifndef ChannelViewCanvas_hpp
#define ChannelViewCanvas_hpp

#include "VisualizerWindowHeaders.h"

namespace ProbeViewer {

enum class RenderMode : int;
enum class ColourSchemeId : int;
struct BitmapRenderTile;

class ChannelViewCanvas : public Component
{
public:
    ChannelViewCanvas(class ProbeViewerCanvas*);
    virtual ~ChannelViewCanvas() override;

    void paint(Graphics&) override;
    void resized() override;

    /**
     *  Perform the necessary work to update if new pixels are ready to be
     *  rendered.
     *
     *  This method is called automatically from:
     *  @see ProbeViewerCanvas::refresh
     */
    void refresh();

    /**
     *  Render each tile for the screen display to the master screen buffer.
     */
    void renderTilesToScreenBufferImage();

    /**
     *  Update the internal state after EACH pixel column is updated during
     *  rendering.
     *
     *  This must be called after each column of pixels on a BitmapRenderTile
     *  is completed to synchronize the currently active drawing tile and
     *  column drawing position within that tile. Failing to call this method
     *  will cause the same column to be repeatedly overwritten offscreen.
     */
    void tick();

    /**
     *  Set the height in pixels of the displayed channels (data and reference)
     */
    void setChannelHeight(float height);

    /**
     *  Returns the height in pixels of the displayed channels.
     */
    float getChannelHeight();

    /**
     *  Receive and queue pixel value updates for RMS, FFT, and SpikeRate
     *  screen images for the given channel.
     *
     *  This method expects the channel to refer to an index within the
     *  data acquisition subset of channels, and does not do bound checking.
     */
    void pushPixelValueForChannel(int channel, float rms, float spikeRate, float fft);

    /**
     *  Set the index of the specific subprocessor for which to render
     *  probe channels.
     */
    void setDisplayedSubprocessor(int subProcessorIdx);

    /**
     *  Returns a pointer the BitmapRenderTile that is currently flagged
     *  for pixel updates.
     */
    BitmapRenderTile* const getFrontBufferPtr() const;

    /**
     *  Get the pixel-wise offset (from the left) of the BitmapRenderTile
     *  returned by ChannelViewCanvas::getFrontBufferPtr().
     *
     *  This value describes the current column that should have its
     *  pixels updated during rendering.
     */
    int getBufferOffsetPosition() const;

    /**
     *  Returns a value describing which RenderMode is currently selected
     *  display on screen (RMS, FFT, or SpikeRate).
     */
    RenderMode getCurrentRenderMode() const;

    /**
     *  Set the RenderMode that should be displayed on screen (FFT, RMS,
     *  or SpikeRate).
     */
    void setCurrentRenderMode(RenderMode r);

    /**
     *  Return a value describing the current user-selected colour
     *  mapping scheme to use for rendering.
     */
    ColourSchemeId getCurrentColourScheme() const;

    /**
     *  Set the current colour mapping that should be used to render
     *  new pixels.
     *
     *  Changing this value does not overwrite old pixels, but will draw
     *  all subsequent pixels with the new mapping.
     */
    void setCurrentColourScheme(ColourSchemeId schemeId);

    OwnedArray<class ProbeChannelDisplay> readSites;
    Array<class ProbeChannelDisplay*> channels;
    Atomic<int> isDirty;
    int numPixelUpdates;

    class CanvasOptionsBar* optionsBar;

    static const int NUM_READ_SITES_FOR_MAX_CHANNELS;
    static const int CHANNEL_DISPLAY_MAX_HEIGHT;
    static const int CHANNEL_DISPLAY_WIDTH;
    static const int CHANNEL_DISPLAY_TILE_WIDTH;

private:
    static const Colour backgroundColour;

    int drawableSubprocessorIdx;

    class ProbeViewerCanvas* canvas;

    float channelHeight;

    ColourSchemeId colourSchemeId;

    Image screenBufferImage;

    RenderMode renderMode;

    OwnedArray<BitmapRenderTile> displayBitmapTiles;

    int frontBackBufferPixelOffset;

    CriticalSection imageMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelViewCanvas);
};

struct BitmapRenderTile
{
    ScopedPointer<Image> rms;
    ScopedPointer<Image> spikeRate;
    ScopedPointer<Image> fft;

    OwnedArray<Array<Image>> readSiteSubImage;

    const int width;
    const int height;

    /**
     *  Create a new BitmapRenderTile with the given dimensions.
     *
     *  The params must not be zero, as this object allocates Image memory
     *  directly from width and height. These values are not checked for
     *  valid input.
     */
    BitmapRenderTile(int width, int height);
    ~BitmapRenderTile() = default;

    /**
     *  Return a pointer to an Image for a complete tile and a specific
     *  RenderMode.
     */
    Image* const getTileForRenderMode(RenderMode mode);

    /**
     *  Return an Image reference that wraps only the given readSite's drawable
     *  area on the currently active front buffer, and for the given RenderMode.
     *
     *  @param readSite     The index of the readsite to draw to
     *  @param mode         The RenderMode which must be drawn to for
     *                      this tile
     */
    Image& getReadSiteSubImageForRenderMode(int readSite, RenderMode mode);

    /**
     *  BitmapRenderTiles are non-copyable and non-moveable.
     */
    BitmapRenderTile(const BitmapRenderTile &) = delete;
    BitmapRenderTile(BitmapRenderTile &&) = delete;
    BitmapRenderTile& operator=(const BitmapRenderTile &) = delete;
    BitmapRenderTile& operator=(BitmapRenderTile &&) = delete;
};

enum class ChannelState : int;

enum class RenderMode : int
{
    RMS,
    SPIKE_RATE,
    FFT
};

class ProbeChannelDisplay : public Component
{
public:
    ProbeChannelDisplay(ChannelViewCanvas* channelsView, class CanvasOptionsBar* optionsBar, ChannelState status, int channelID, int readSiteID, float sampleRate);
    virtual ~ProbeChannelDisplay() override;

    void paint(Graphics& g) override;

    //void pxPaint(Image::BitmapData *bitmapData);
    void pxPaint();

    /**
     *  Return a value describing whether this channel represents a refer-
     *  ence node or a data acquisition node.
     */
    ChannelState getChannelState() const;

    /**
     *  Set this channel's state to reflect whether it is a reference node
     *  or an active data acquiring node.
     */
    void setChannelState(ChannelState status);

    /**
     *  Accept and queue one pixel worth of updates on this channel for
     *  RMS, FFT, and SpikeRate values.
     */
    void pushSamples(float rms, float spikeRate, float fft);

    /**
     *  Return the index number of this channel relative to the subset of
     *  available probe channels that are data acquiring.
     */
    int getChannelId() const;

    /**
     *  Set the index of this channel relative to the subset of available
     *  probe channels that are only data acquiring.
     */
    void setChannelId(int id);

    /**
     *  Return the index number of this channel, relative to the
     *  complete set of channels available on the probe (including
     *  reference nodes).
     */
    int getReadSiteId() const;

    /**
     *  Set the index relative to data and reference nodes to the given
     *  param
     */
    void setReadSiteId(int id);

    /**
     *  Return the sample rate of this channel, or 0 if this is a
     *  reference channel with no data stream.
     */
    float getSampleRate();

    /**
     *  Return the number of samples used to calculate each pixel for
     *  this channel.
     */
    int getNumSamplesPerPixel();

private:
    ChannelViewCanvas* channelsView;
    CanvasOptionsBar* optionsBar;

    float sampleRate;
    int samplesPerPixel;

    ChannelState channelState;
    int channelID;
    int readSiteID;

    int yBitmapPos;

    Array<float> rms;
    Array<float> spikeRate;
    Array<float> fft;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeChannelDisplay);
};

}



#endif /* ChannelViewCanvas_hpp */
