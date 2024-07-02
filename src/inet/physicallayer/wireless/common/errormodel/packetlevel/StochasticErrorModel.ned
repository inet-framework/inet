//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.errormodel.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.ErrorModelBase;

//
// This error model provides parameters to specify the constant packet error
// rate, bit error rate, and symbol error rate for receptions independent of
// any interfering transmission or noise.
//
module StochasticErrorModel extends ErrorModelBase
{
    parameters:
        double packetErrorRate = default(nan); // A probability in the range [0, 1] or NaN if not set
        double bitErrorRate = default(nan);    // A probability in the range [0, 1] or NaN if not set
        double symbolErrorRate = default(nan); // A probability in the range [0, 1] or NaN if not set
        @class(StochasticErrorModel);
}

