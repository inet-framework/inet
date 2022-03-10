//
// Copyright (C) 2014 Florian Meier <florian.meier@tu-harburg.de>
// Copyright (C) 2006 Isabel Dietrich <isabel.dietrich@informatik.uni-erlangen.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.mobility.static;

import inet.mobility.base.MobilityBase;

//
// Places all hosts on concentric circles
//
//# TODO rename to StaticConcentricCircleMobility?
simple StaticConcentricMobility extends MobilityBase
{
    parameters:
        int numHosts;
        double initialZ @unit(m) = default(0m);
        double distance @unit(m) = default(100m);

        @class(StaticConcentricMobility);
}

