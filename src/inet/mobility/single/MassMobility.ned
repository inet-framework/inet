//
// Copyright (C) 2005 Emin Ilker Cetinbas, Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Generalization: Andras Varga
//

package inet.mobility.single;

import inet.mobility.base.MovingMobilityBase;

//
// This is a random mobility model for a mobile host with a mass. It is the
// one used in "Optimized Smooth Handoffs in Mobile IP" by Perkins &Wang.
//
// "An MH moves within the room according to the following pattern. It moves
// along a straight line for a certain period of time before it makes a turn.
// This moving period is a random number, normally distributed with average of
// 5 seconds and standard deviation of 0.1 second. When it makes a turn, the
// new direction (angle) in which it will move is a normally distributed
// random number with average equal to the previous direction and standard
// deviation of 30 degrees. Its speed is also a normally distributed random
// number, with a controlled average, ranging from 0.1 to 0.45 (unit/sec), and
// standard deviation of 0.01 (unit/sec). A new such random number is picked
// as its speed when it makes a turn. This pattern of mobility is intended to
// model node movement during which the nodes have momentum, and thus do not
// start, stop, or turn abruptly. When it hits a wall, it reflects off the
// wall at the same angle; in our simulated world, there is little other
// choice."
//
// This implementation can be parameterized a bit more, via the changeInterval
// and angleDelta parameters. The parameters described above
// correspond to the following settings:
//    - changeInterval = normal(5, 0.1)
//    - angleDelta = normal(0, 30)
//    - speed = normal(avgSpeed, 0.01)
//
// 3D (spatial movement) is also supported using rotationAxisAngle parameter.
//
// @author Emin Ilker Cetinbas
//
simple MassMobility extends MovingMobilityBase
{
    parameters:
        double initialX @unit(m) = default(uniform(constraintAreaMinX, constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(constraintAreaMinY, constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(constraintAreaMinZ, constraintAreaMaxZ)));
        double initialMovementHeading @unit(deg) = default(uniform(0deg, 360deg)); // initial heading
        double initialMovementElevation @unit(deg) = default(0deg); // initial elevation
        bool initFromDisplayString = default(true); // enables one time initialization from the subject module's display string
        volatile double changeInterval @unit(s); // frequency of changing speed and angle (can be random)
        volatile double angleDelta @unit(deg); // rotate velocity vector by this much around the rotation axis defined by the rotationAxisAngle parameter (can be random)
        volatile double rotationAxisAngle @unit(deg) = default(0deg); // defines a vector in the plane perpendicular to the velocity, where 0 deg means the Z axis when bank is zero (can be random)
        volatile double speed @mutable @unit(mps); // speed (can be random, updated every changeInterval)
        @class(MassMobility);
}

