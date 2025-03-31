//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.physicallayer.wireless.common.errormodel.bitlevel;

import inet.physicallayer.wireless.common.base.bitlevel.LayeredErrorModelBase;

//
// Implements a stochastic error model for wireless communications that applies
// random errors at different protocol layers based on configurable error rates.
// Supports bit-level and symbol-level error modeling with specified error
// probabilities, allowing simulation of transmission errors in wireless channels.
//
module StochasticLayeredErrorModel extends LayeredErrorModelBase
{
    parameters:
        double packetErrorRate = default(nan);
        double bitErrorRate = default(nan);
        double symbolErrorRate = default(nan);
        @class(StochasticLayeredErrorModel);
}
