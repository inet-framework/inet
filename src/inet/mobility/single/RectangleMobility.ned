//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Moves the node around a rectangle.
//
simple RectangleMobility extends MovingMobilityBase
{
    parameters:
        double startPos; // in range [0.0,4.0): topleft=0, topright=1, bottomright=2, bottomleft=3
        double speed @unit(mps); // speed of the host
        @class(RectangleMobility);
}

