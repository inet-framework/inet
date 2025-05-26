//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.pathloss;

//
// Models Rayleigh fading in wireless channels, which occurs when signals
// propagate through multipath environments with no dominant line-of-sight path.
// This model extends free space path loss with stochastic variations based on
// the Rayleigh distribution, making it suitable for dense urban and indoor
// environments. Represents a special case of Rician fading with no dominant
// signal component, providing realistic signal strength fluctuations for
// non-line-of-sight wireless communication scenarios.
//
module RayleighFading extends FreeSpacePathLoss
{
    parameters:
        @class(RayleighFading);
}

