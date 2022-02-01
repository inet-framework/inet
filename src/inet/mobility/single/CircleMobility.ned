//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Moves the node around a circle.
//
simple CircleMobility extends MovingMobilityBase
{
    parameters:
        double cx @unit(m); // x coord of the center of the circle
        double cy @unit(m); // y coord of the center of the circle
        double cz @unit(m) = default(0m); // z coord of the center of the circle
        double r @unit(m); // radius of the circle
        double speed @unit(mps) = default(0mps); // speed of the host
        double startAngle @unit(deg) = default(uniform(0deg, 360deg)); // starting angle
        @class(CircleMobility);
}

