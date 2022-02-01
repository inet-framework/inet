//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.IRx;

//
// The default implementation of ~IRx.
//
simple Rx like IRx
{
    parameters:
        @display("i=block/rx");
        @signal[navChanged](type=simtime_t);
        @signal[packetDropped](type=inet::Packet);
        @statistic[nav](title="network allocation vector"; source=navChanged; record=vector; interpolationmode=linear);
}

