//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// A mobility model that simulates a vehicle turning at defined waypoints.
//
// Waypoints are defined in a text file as x,y,z with one waypoint per line.
// When all waypoints have been reached, it loops back to the first one.
//

simple VehicleMobility extends MovingMobilityBase
{
    parameters:
        @class(VehicleMobility);
        string waypointFile; // contains the waypoints as one waypoint x,y,z per line
        double waypointProximity @unit(m); // a waypoint is reached when it is less than waypointProximity meters away
        double speed @unit(mps);
        string groundModule = default("environment.ground"); // module path of the ground module
        double initialX @unit(m) = default(uniform(constraintAreaMinX, constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(constraintAreaMinY, constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(constraintAreaMinZ, constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
}

