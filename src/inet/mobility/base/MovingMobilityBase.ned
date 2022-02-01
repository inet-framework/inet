//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.base;

//
// Abstract base module for mobility models.
//
simple MovingMobilityBase extends MobilityBase
{
    parameters:
        double updateInterval @unit(s) = default(0.1s); // the simulation time interval used to regularly signal mobility state changes and update the display
        bool faceForward = default(true);
}

