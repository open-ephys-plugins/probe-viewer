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

#include "AverageView.hpp"

#include "ChannelViewCanvas.hpp"

using namespace ProbeViewer;

const int AVERAGE_VIEW_WIDTH = 300;

AverageView::AverageView(ChannelViewCanvas* canvas) :

    canvas(canvas),
    numChannels(1),
    numTrials(0),
    sampleIndex(0),
    channelHeight(10),
    screenBufferImage(Image::RGB, AVERAGE_VIEW_WIDTH, 1, true)
{

}


void AverageView::updateViewSettings()
{
    numChannels = canvas->getNumChannels();

    if (numChannels == 0)
        numChannels = 1;

    screenBufferImage = Image(Image::RGB, AVERAGE_VIEW_WIDTH, numChannels * channelHeight, true);

	screenBuffer.setSize(numChannels, AVERAGE_VIEW_WIDTH);

    numTrials = 0;
    sampleIndex = 0;

    Graphics g(screenBufferImage);
	g.setGradientFill(ColourGradient(Colours::orange, 0, 0, Colours::white, 0, numChannels, false));
    g.fillRect(0, 0, AVERAGE_VIEW_WIDTH, int(numChannels * channelHeight));
}


bool AverageView::pushPixelValueForChannel(int channel, float value)
{
    screenBuffer.addSample(channel, sampleIndex, value);

    if ((channel == numChannels - 1))
    {
        sampleIndex += 1;

        if (sampleIndex == AVERAGE_VIEW_WIDTH)
        {
            repaint();
            sampleIndex = 0;
            numTrials += 1;
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

void AverageView::paint(Graphics& g)
{
    // scale the values in the screen buffer to the image

    const float verticalScale = float(channelHeight * numChannels) / numChannels  *2;
    const float horizontalScale = getWidth() / float(AVERAGE_VIEW_WIDTH);

    //std::cout << "AverageView verticalScale: " << verticalScale << ", height: " << numChannels * 2 << std::endl;
    
    const auto transform = AffineTransform::scale(horizontalScale, verticalScale);

    g.drawImageTransformed(screenBufferImage, transform);

}

void AverageView::setChannelHeight(float height)
{
    channelHeight = height;

    repaint();
}
