//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MobilityBase;
import inet.mobility.contract.IMobility;

//
// Combines the trajectory of several other mobility modules using superposition.
// In other words, the position, velocity, and acceleration
// are the sum of the respective quantities of all submodules.
//
// This module allows forming a group mobility model by using a shared mobility
// component in the mobility of each group member. It also allows separating the
// mobility model of initial positioning from positioning during the simulation.
// It also makes it possible to separate the mobility model of positioning from the
// mobility model of orientation.
//
module SuperpositioningMobility extends MobilityBase
{
    parameters:
        int numElements; // Number of combined mobility modules
        string positionComposition @enum("zero", "sum", "average") = default("sum");
        string orientationComposition @enum("zero", "sum", "average", "faceForward") = default("sum");
        element[*].subjectModule = default(""); // Disable visual subject module of superposition elements by default
        @class(SuperpositioningMobility);
    submodules:
        element[numElements]: <> like IMobility {
            parameters:
                @display("p=100,100,row,150");
        }
}

