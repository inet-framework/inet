//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

//
// Implements the Nakagami fading model for wireless signal propagation. This
// statistical model extends the free space path loss model by adding multipath
// fading effects. The shapeFactor parameter determines the severity of fading,
// with lower values representing more severe fading conditions. When shapeFactor
// equals 1, the Nakagami distribution reduces to the Rayleigh distribution,
// modeling severe multipath with no line-of-sight component.
//
// @see RayleighFading
//
module NakagamiFading extends FreeSpacePathLoss
{
    parameters:
        double shapeFactor = default(1);
        @class(NakagamiFading);
}

