//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.ITx;

//
// Responsible for unconditionally transmitting a frame after waiting for
// a specified inter-frame space. This is the default implementation of the
// ~ITx module interface.
//
simple Tx extends SimpleModule like ITx
{
    parameters:
        @class(Tx);
        string rxModule;
        @display("i=block/tx");
}

