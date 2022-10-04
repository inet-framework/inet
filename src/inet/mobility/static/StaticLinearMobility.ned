//
// Copyright (C) 2012 Alfonso Ariza, Universidad de Malaga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.mobility.static;

//
// Mobility model which places all hosts at constant distances
// in a line with an orientation
//
// @author Alfonso Ariza
//
simple StaticLinearMobility extends StationaryMobility
{
    parameters:
        double separation @unit(m);
        double orientation @unit(deg) = default(0deg);
        @class(StaticLinearMobility);
}

