//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

//
// Implements the two-ray ground reflection path loss model for wireless
// communications. This model accounts for both the direct line-of-sight path and
// a ground-reflected path between transmitter and receiver. For distances less
// than the crossover distance, it uses the free space model; beyond that, it
// applies the two-ray formula which considers transmitter and receiver heights
// above ground. (The crossover distance is computed from the heights and
// the frequency.) This model provides more accurate predictions than free space
// models for medium to long distances in environments with significant ground
// reflections.
//
// @see TwoRayInterference
//
module TwoRayGroundReflection extends FreeSpacePathLoss
{
    parameters:
        @class(TwoRayGroundReflection);
        string physicalEnvironmentModule = default("physicalEnvironment"); // The path of the environment model relative to the network
}

