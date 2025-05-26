//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

//
// Models Rician fading in wireless channels, which occurs when a signal reaches
// the receiver through both direct line-of-sight and multipath propagation. The
// k parameter (in dB) controls the ratio between the direct and scattered signal
// components, with higher values indicating stronger line-of-sight conditions.
// This model extends free space path loss with stochastic variations based on
// the Rician distribution, making it suitable for indoor and urban environments
// where a dominant signal path exists alongside weaker multipath components.
//
module RicianFading extends FreeSpacePathLoss
{
    parameters:
        double k @unit(dB) = default(8dB);
        @class(RicianFading);
}

