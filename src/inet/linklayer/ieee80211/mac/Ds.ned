//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IDs;

//
// Implements the DS (Distribution Service) for IEEE 802.11, which is
// responsible for distributing correctly received frames to the higher
// layer, to the wireless LAN, etc.
//
// This is the default implementation of the ~IDs module interface.
//
simple Ds extends SimpleModule like IDs
{
    parameters:
        @class(Ds);
        string mibModule;
        @display("i=block/routing");
        @signal[packetDropped];
}

