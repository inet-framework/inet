//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contention;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.ICollisionController;

//
// A collision controller used with EDCA. It detects and reports internal
// collisions among ~Edcaf instances.
//
simple EdcaCollisionController extends SimpleModule like ICollisionController
{
    parameters:
        @class(EdcaCollisionController);
        @display("i=block/switch");
}

