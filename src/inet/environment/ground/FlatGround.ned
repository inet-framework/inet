//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.environment.contract.IGround;

//
// Models a flat ground surface with a configurable elevation. Provides methods
// to compute the ground projection of a 3D position by setting its z-coordinate
// to the elevation value, and to compute the ground normal which is always
// pointing upward (0,0,1) for a flat surface.
//
module FlatGround like IGround
{
    parameters:
        double elevation @unit(m) = default(0m);
        @class(FlatGround);
}
