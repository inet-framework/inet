//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IRtsPolicy;


//
// Implements the IEEE 802.11e/n/ac QoS-aware Request-to-Send (RTS) policy for 
// Quality of Service stations. This policy extends the standard RTS functionality 
// with QoS considerations, determining when to use the RTS/CTS handshake before 
// transmitting QoS data frames. Like the basic RTS policy, it uses a configurable 
// threshold to decide when to send RTS frames, but additionally considers traffic 
// categories and access categories when making decisions. This helps prioritize 
// medium access for different types of traffic in QoS-enabled networks.
//
// @see ~IRtsPolicy, ~RtsPolicy, ~QosCtsPolicy
//
simple QosRtsPolicy like IRtsPolicy
{
    parameters:
        string rateSelectionModule;
        double ctsTimeout @unit(s) = default(-1s);
        int rtsThreshold @unit(B) = default(2346B);
        @display("i=block/control");
}
