//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.errormodel.bitlevel;

import inet.physicallayer.wireless.common.base.bitlevel.LayeredErrorModelBase;

module StochasticLayeredErrorModel extends LayeredErrorModelBase
{
    parameters:
        double packetErrorRate = default(nan);
        double bitErrorRate = default(nan);
        double symbolErrorRate = default(nan);
        @class(StochasticLayeredErrorModel);
}

