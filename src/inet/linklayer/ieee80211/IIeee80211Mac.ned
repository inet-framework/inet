//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211;

import inet.linklayer.contract.IMacProtocol;

//
// Module interface for the IEEE 802.11 MAC module type.
//
moduleinterface IIeee80211Mac extends IMacProtocol
{
    parameters:
        @display("i=block/layer");
    gates:
        input mgmtIn;
        output mgmtOut;
}

