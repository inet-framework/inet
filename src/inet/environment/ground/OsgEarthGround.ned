//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.environment.contract.IGround;

module OsgEarthGround like IGround
{
    parameters:
        @class(OsgEarthGround);
        string osgEarthSceneVisualizerModule = default("");
        string coordinateSystemModule = default("");
}

