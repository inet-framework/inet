//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IRtsPolicy;


//
// Implements the standard IEEE 802.11 Request-to-Send (RTS) policy that determines 
// when to use the RTS/CTS handshake before data frame transmission. This policy 
// uses a configurable threshold (rtsThreshold) to decide whether to send an RTS 
// frame - frames larger than this threshold will trigger the RTS/CTS exchange. 
// The policy helps mitigate hidden node problems by reserving the medium before 
// transmitting large data frames, reducing collision probability. It also manages 
// the timeout period for expected CTS responses.
//
// @see ~IRtsPolicy, ~QosRtsPolicy, ~CtsPolicy
//
simple RtsPolicy like IRtsPolicy
{
    parameters:
        string rateSelectionModule;
        double ctsTimeout @unit(s) = default(-1s);
        int rtsThreshold @unit(B) = default(2346B);
        @display("i=block/control");
}
