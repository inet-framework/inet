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
        double startPos; // In range [0.0, 4.0): top left=0, top right=1, bottom right=2, bottom left=3
        double speed @unit(mps); // Speed of the host
        @class(RectangleMobility);
}

