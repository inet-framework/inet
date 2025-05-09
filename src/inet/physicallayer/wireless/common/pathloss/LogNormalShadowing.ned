//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

//
// Path loss model that combines free space path loss with log-normal shadowing.
// Extends the deterministic free space model by adding a random component that
// follows a normal distribution with zero mean and configurable standard
// deviation (sigma). This accounts for signal variations due to obstacles and
// environmental factors, providing more realistic wireless signal propagation
// modeling than pure deterministic models.
//
module LogNormalShadowing extends FreeSpacePathLoss
{
    parameters:
        double sigma = default(1);
        @class(LogNormalShadowing);
}

