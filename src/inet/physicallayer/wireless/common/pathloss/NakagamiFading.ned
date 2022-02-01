//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

module NakagamiFading extends FreeSpacePathLoss
{
    parameters:
        double shapeFactor = default(1);
        @class(NakagamiFading);
}

