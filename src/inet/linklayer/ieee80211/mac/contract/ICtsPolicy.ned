//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 Clear-to-Send (CTS) policies that determine how to 
// respond to Request-to-Send (RTS) frames. CTS policies decide whether to send 
// a CTS frame in response to a received RTS frame based on factors such as the 
// station's current state, NAV (Network Allocation Vector) settings, and other 
// medium access conditions. The CTS response completes the RTS/CTS handshake, 
// reserving the medium for the subsequent data frame transmission and helping 
// prevent collisions from hidden nodes.
//
// @see ~CtsPolicy, ~QosCtsPolicy, ~IRtsPolicy
//
moduleinterface ICtsPolicy
{
    parameters:
        string rxModule;
        string rateSelectionModule;
        @display("i=block/control");
}
