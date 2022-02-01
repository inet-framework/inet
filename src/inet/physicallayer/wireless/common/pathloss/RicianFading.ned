//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

module RicianFading extends FreeSpacePathLoss
{
    parameters:
        double k @unit(dB) = default(8dB);
        @class(RicianFading);
}

