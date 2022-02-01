//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contention;

import inet.linklayer.ieee80211.mac.contract.ICollisionController;

simple EdcaCollisionController like ICollisionController
{
    parameters:
        @display("i=block/switch");
}

