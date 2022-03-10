//
// Copyright (C) 2005 Georg Lutz, Institut fuer Telematik, University of Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Implements the Random Waypoint mobility model.
//
// The node moves in line segments. For each line segment, a random destination
// position (distributed uniformly over the constraint area or volume) and a
// random speed is chosen. You can define a speed as a variate from which a new
// value will be drawn for each line segment; it is customary to specify it as
// uniform(minSpeed, maxSpeed). When the node reaches the target position,
// it waits for the time waitTime which can also be defined as a variate.
// After this time the the algorithm calculates a new random position, etc.
//
// This model was originally written by Georg Lutz (GeorgLutz AT gmx DOT de) for
// his diploma thesis "Effizientes Modell fuer Funkverbindungen in 4G-Netzen fuer
// OMNeT++" (Efficient model for radio links in 4G networks for OMNeT++)
// at Institut fuer Telematik, Universitaet Karlsruhe (Institute for
// Telematics, University Karlsruhe, Germany), 2005-06-21.
//
// @author Georg Lutz
//
simple RandomWaypointMobility extends MovingMobilityBase
{
    parameters:
        double initialX @unit(m) = default(uniform(this.constraintAreaMinX, this.constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(this.constraintAreaMinY, this.constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(this.constraintAreaMinZ, this.constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
        volatile double speed @unit(mps) = default(mps); // use uniform(minSpeed, maxSpeed) or another distribution
        volatile double waitTime @unit(s) = default(0s); // wait time between reaching a target and choosing a new one
        @class(RandomWaypointMobility);
}

