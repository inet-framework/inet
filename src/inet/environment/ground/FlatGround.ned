//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.environment.ground;

import inet.environment.contract.IGround;

module FlatGround like IGround
{
    parameters:
        double elevation @unit(m) = default(0m);
        @class(FlatGround);
}

