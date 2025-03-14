//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.linklayer.ieee80211.mac.contract.ICtsPolicy;


//
// Implements the IEEE 802.11e/n/ac QoS-aware Clear-to-Send (CTS) policy for 
// Quality of Service stations. This policy extends the standard CTS functionality 
// with QoS considerations, determining how to respond to Request-to-Send (RTS) 
// frames in QoS-enabled networks. It evaluates received RTS frames and decides 
// whether to send a CTS response based on the station's current state, NAV settings, 
// traffic categories, and medium conditions. The policy helps prioritize medium 
// access for different types of traffic while maintaining the collision avoidance 
// benefits of the RTS/CTS handshake.
//
// @see ~ICtsPolicy, ~CtsPolicy, ~QosRtsPolicy
//
simple QosCtsPolicy like ICtsPolicy
{
    parameters:
        string rxModule;
        string rateSelectionModule;
        @display("i=block/control");
}
