//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.common.CompoundModule;
import inet.environment.contract.IGround;

//
// Models a ground surface using OsgEarth's elevation data. Provides methods to
// compute ground projections and normals based on real-world terrain information.
// Requires OsgEarth integration to be enabled during compilation and depends on
// a SceneOsgEarthVisualizer for accessing the map and a coordinate system for
// geographic conversions.
//
module OsgEarthGround extends CompoundModule like IGround
{
    parameters:
        @class(OsgEarthGround);
        string osgEarthSceneVisualizerModule = default("");
        string coordinateSystemModule = default("");
}
