//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 receiver components that process incoming frames and manage 
// channel state information. Defines functionality for frame error checking, Network 
// Allocation Vector (NAV) management, and channel state notification. Implementations 
// combine physical carrier sense (Clear Channel Assessment) with virtual carrier sense 
// (NAV) to determine medium availability, forwarding valid frames while discarding 
// corrupted ones.
//
// @see ~Rx
//
moduleinterface IRx
{
    parameters:
        @display("i=block/rx");
}
