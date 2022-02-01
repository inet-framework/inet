//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.mobility.single;

import inet.mobility.base.MobilityBase;

//
// This module provides a mobility that is attached to another mobility at a
// given offset. The position, velocity and acceleration are all affectred by
// the respective quantites and the orientation of the mobility where this one
// is attached.
//
simple AttachedMobility extends MobilityBase
{
    parameters:
        string mobilityModule;
        double offsetX @unit(m) = default(0m);
        double offsetY @unit(m) = default(0m);
        double offsetZ @unit(m) = default(0m);
        double offsetHeading @unit(deg) = default(0deg);
        double offsetElevation @unit(deg) = default(0deg);
        double offsetBank @unit(deg) = default(0deg);
        @class(AttachedMobility);
}

