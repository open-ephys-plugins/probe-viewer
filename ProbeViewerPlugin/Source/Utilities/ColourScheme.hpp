//
//  ColourScheme.hpp
//  ProbeViewerPlugin
//
//  Created by Kelly Fox on 10/16/17.
//  Copyright © 2017 Allen Institute. All rights reserved.
//

#ifndef ColourScheme_hpp
#define ColourScheme_hpp

#include <VisualizerWindowHeaders.h>

namespace ProbeViewer {
    
enum class ColourSchemeId : int
{
    INFERNO,
    VIRIDIS,
    PLASMA,
    MAGMA,
    JET
};
    
namespace ColourScheme
{
    
    Colour getColourForNormalizedValue(float val);
    Colour getColourForNormalizedValueInScheme(float val, ColourSchemeId colourScheme);
    
    void setColourScheme(ColourSchemeId colourScheme);
};
};

#endif /* ColourScheme_hpp */
