/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Open Ephys
 
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
    ProbeViewerCanvas();
    
    virtual ~ProbeViewerCanvas() override {}
    
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
    virtual void setParameter(int, float) override {}
    
    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, int, int, float) override {}
};
    
}

#endif /* __PROBEVIEWERCANVAS_H__ */
