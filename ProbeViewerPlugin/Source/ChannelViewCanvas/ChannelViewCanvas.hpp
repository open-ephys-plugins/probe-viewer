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

#include <VisualizerWindowHeaders.h>

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
    
    void refresh();
    
    void renderTilesToScreenBufferImage();
    
    void tick();
    
    void setChannelHeight(float height);
    float getChannelHeight();
    
    void updateRMS(int channel, float value);
    void pushPixelValueForChannel(int channel, float rms, float spikeRate, float fft);
    
    void setDisplayedSubprocessor(int subProcessorIdx);
    
    BitmapRenderTile* const getFrontBufferPtr() const;
    
    int getBufferOffsetPosition() const;
    
    RenderMode getCurrentRenderMode() const;
    void setCurrentRenderMode(RenderMode r);
    
    ColourSchemeId getCurrentColourScheme() const;
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
    
    size_t drawableSubprocessorIdx;
    
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
    
    BitmapRenderTile(int width, int height);
    ~BitmapRenderTile() = default;
    
    Image* const getTileForRenderMode(RenderMode mode);
    Image& getReadSiteSubImageForRenderMode(int readSite, RenderMode mode);
    
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
    
    ChannelState getChannelState() const;
    void setChannelState(ChannelState status);
    
    void pushSamples(float rms, float spikeRate, float fft);
    
    int getChannelId() const;
    void setChannelId(int id);
    
    int getReadSiteId() const;
    void setReadSiteId(int id);
    
    float getSampleRate();
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeChannelDisplay);
};

}



#endif /* ChannelViewCanvas_hpp */
