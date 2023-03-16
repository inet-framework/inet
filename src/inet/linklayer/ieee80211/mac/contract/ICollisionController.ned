//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for collision controllers. A collision controller is used
// with EDCA, and it detects and reports internal collisions among
// ~Edcaf instances.
//
moduleinterface ICollisionController
{
    parameters:
        @display("i=block/switch");
}

