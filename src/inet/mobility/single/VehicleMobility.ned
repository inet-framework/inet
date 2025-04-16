//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// Mobility model for ground vehicles that follow waypoints with realistic
// turning behavior. Reads waypoints from a file and moves the vehicle along
// them at a specified speed, calculating appropriate angular velocity for turns.
// Supports terrain following by projecting positions onto the ground and
// orienting the vehicle according to the ground normal.
//
// Waypoints are defined in a text file as x,y,z with one waypoint per line.
// When all waypoints have been reached, it loops back to the first one.
//
simple VehicleMobility extends MovingMobilityBase
{
    parameters:
        @class(VehicleMobility);
        string waypointFile; // Contains the waypoints as one waypoint x,y,z per line
        double waypointProximity @unit(m); // A waypoint is reached when it is less than waypointProximity meters away
        double speed @unit(mps);
        string groundModule = default("environment.ground"); // Module path of the ground module
        double initialX @unit(m) = default(uniform(constraintAreaMinX, constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(constraintAreaMinY, constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(constraintAreaMinZ, constraintAreaMaxZ)));
        bool initFromDisplayString = default(true); // Enables one-time initialization from the subject module's display string
}
