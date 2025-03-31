//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.pathloss;

//
// Implements the Nakagami fading model for wireless signal propagation. This
// statistical model extends the free space path loss model by adding multipath
// fading effects. The shapeFactor parameter controls the severity of fading,
// with lower values representing more severe fading conditions. When shapeFactor
// equals 1, the model corresponds to Rayleigh fading.
//
module NakagamiFading extends FreeSpacePathLoss
{
    parameters:
        double shapeFactor = default(1);
        @class(NakagamiFading);
}
