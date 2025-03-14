//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.linklayer.ieee80211.mac.contract.ICtsPolicy;


//
// Implements the standard IEEE 802.11 Clear-to-Send (CTS) policy that determines 
// how to respond to Request-to-Send (RTS) frames. This policy evaluates received 
// RTS frames and decides whether to send a CTS response based on the station's 
// current state, NAV settings, and medium conditions. When a valid RTS is received 
// and conditions permit, it generates a CTS frame with appropriate duration field 
// to reserve the medium for the upcoming data transmission. This helps complete 
// the RTS/CTS handshake that protects against hidden node problems.
//
// @see ~ICtsPolicy, ~QosCtsPolicy, ~RtsPolicy
//
simple CtsPolicy like ICtsPolicy
{
    parameters:
        string rxModule;
        string rateSelectionModule;
        @display("i=block/control");
}
