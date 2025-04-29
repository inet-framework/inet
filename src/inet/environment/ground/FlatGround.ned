//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.common.CompoundModule;
import inet.environment.contract.IGround;

module FlatGround extends CompoundModule like IGround
{
    parameters:
        double elevation @unit(m) = default(0m);
        @class(FlatGround);
}

