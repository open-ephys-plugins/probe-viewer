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
     *  Receive and queue pixel value updates for RMS, FFT, or SpikeRate
     *  screen image for the given channel.
     *
     *  This method expects the channel to refer to an index within the
     *  data acquisition subset of channels, and does not do bound checking.
     */
    void pushPixelValueForChannel(int channel, float value);

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

    /**
     *  Set number of channels for the view
     *  and regenrate Bitmap render tiles
     */
    void updateViewSettings();

    OwnedArray<class ProbeChannelDisplay> channels;
    Atomic<int> isDirty;
    int numPixelUpdates;

    class CanvasOptionsBar* optionsBar;

    static const int CHANNEL_DISPLAY_MAX_HEIGHT;
    static const int CHANNEL_DISPLAY_WIDTH;
    static const int CHANNEL_DISPLAY_TILE_WIDTH;

private:
    static const Colour backgroundColour;

    int drawableSubprocessorIdx;
    int numChannels;

    class ProbeViewerCanvas* canvas;

    float channelHeight;

    ColourSchemeId colourSchemeId;

    Image screenBufferImage;

    RenderMode renderMode;

    OwnedArray<BitmapRenderTile> displayBitmapTiles;

    int frontBackBufferPixelOffset;
    int frontBufferIndex;
    
    bool fullRedraw;

    CriticalSection imageMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelViewCanvas);
};

struct BitmapRenderTile
{
    Image renderImage;

    Array<Image> channelSubImage;

    const int width;
    const int height;
    const int numChannels;

    /**
     *  Create a new BitmapRenderTile with the given dimensions.
     *
     *  The params must not be zero, as this object allocates Image memory
     *  directly from width, height, and number of channels. These values are not checked for
     *  valid input.
     */
    BitmapRenderTile(int width, int height, int numChannels);
    ~BitmapRenderTile() = default;

    /**
     *  Return a pointer to an Image for a complete tile and a specific
     *  RenderMode.
     */
    Image* const getTile();

    /**
     *  Return an Image reference that wraps only the given channel's drawable
     *  area on the currently active front buffer, and for the given RenderMode.
     *
     *  @param channel      The index of the channel to draw to
     *  @param mode         The RenderMode which must be drawn to for
     *                      this tile
     */
    Image getChannelSubImage(int channel);

    /**
     *  BitmapRenderTiles are non-copyable and non-moveable.
     */
    BitmapRenderTile(const BitmapRenderTile &) = delete;
    BitmapRenderTile(BitmapRenderTile &&) = delete;
    BitmapRenderTile& operator=(const BitmapRenderTile &) = delete;
    BitmapRenderTile& operator=(BitmapRenderTile &&) = delete;
};

enum class RenderMode : int
{
    RMS,
    SPIKE_RATE,
    FFT
};

/** 
    
    Renders the data for a single channel

*/
class ProbeChannelDisplay : public Component
{
public:
    /** Constructor */
    ProbeChannelDisplay(ChannelViewCanvas* channelsView, 
                        class CanvasOptionsBar* optionsBar, 
                        int channelID, 
                        float sampleRate);
    
    /** Destructor*/
    virtual ~ProbeChannelDisplay() override;

	/** Paints the channel display */
    void pxPaint();

    /**
     *  Accept and queue one pixel worth of updates on this channel for
     *  RMS, FFT, or SpikeRate value.
     */
    void pushSample(float sample);

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

    int samplesPerPixel;
    float sampleRate;
    int channelID;

    Array<float> samples;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeChannelDisplay);
};

}



#endif /* ChannelViewCanvas_hpp */
