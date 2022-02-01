//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.ITx;

//
// The default implementation of ~ITx.
//
simple Tx like ITx
{
    parameters:
        string rxModule;
        @display("i=block/tx");
}

