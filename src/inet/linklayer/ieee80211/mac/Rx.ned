//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.IRx;

//
// Receiver component for IEEE 802.11 that processes incoming frames from the physical layer. 
// Performs error checking on received frames, manages the Network Allocation Vector (NAV) 
// for virtual carrier sensing, and provides channel state information to other MAC components. 
// Maintains timing information for medium access control and implements the virtual carrier 
// sensing mechanism essential for CSMA/CA operation.
//
// @see ~IRx
//
simple Rx like IRx
{
    parameters:
        @display("i=block/rx");
        @signal[navChanged](type=simtime_t);
        @signal[packetDropped](type=inet::Packet);
        @statistic[nav](title="network allocation vector"; source=navChanged; record=vector; interpolationmode=linear);
}
