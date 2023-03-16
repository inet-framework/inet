//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.ITx;

//
// Responsible for unconditionally transmitting a frame after waiting for
// a specified inter-frame space. This is the default implementation of ~ITx.
//
simple Tx like ITx
{
    parameters:
        string rxModule;
        @display("i=block/tx");
}

