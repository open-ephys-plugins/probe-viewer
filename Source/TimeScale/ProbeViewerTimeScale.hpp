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

#ifndef ProbeViewerTimeScale_hpp
#define ProbeViewerTimeScale_hpp

#include "VisualizerWindowHeaders.h"

namespace ProbeViewer {

/** 
    Generic timescale display
*/
class TimeScale : public Component
{
public:
    /** Constructor */
	TimeScale();

    /** Destructor */
	virtual ~TimeScale() override { }
    
	/** Render the timescale */
	void paint(Graphics& g) override;

	/** Set window */
	void setWindowSize(float preSeconds, float postSeconds);

private:
    float preSeconds = 0.0f;
    float postSeconds = 10.0f;
    float resolution = 0.5;
    
    Font font;
};

/**

	Displays a timescale for the ProbeViewerCanvas

*/
class ProbeViewerTimeScale : public Component
{
public:

    /** Constructor */
    ProbeViewerTimeScale();

    /** Destructor */
    virtual ~ProbeViewerTimeScale() override { }

    /** Called when clicked */
    void resized() override;

    /** Sets the window size (in seconds) of the rolling view*/
    void setRollingViewWindowSize(float windowSize);

    /** Sets the window size (in seconds) of the average view (pre and post)*/
    void setAverageViewWindowSize(float preWindow, float postWindow);

    /** Shows/hides the average view timescale */
    void showAverageView(bool show);

    /** Set distance between edge of canvas and start of timescale */
    void setMarginOffset(float marginOffset);

private:

    std::unique_ptr<TimeScale> rollingViewTimeScale;
    std::unique_ptr<TimeScale> averageViewTimeScale;

    float margin = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProbeViewerTimeScale);
};

};

#endif /* ProbeViewerTimeScale_hpp */
