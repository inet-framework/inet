//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac;

import inet.linklayer.ieee80211.mac.contract.ITx;

//
// Transmitter component for IEEE 802.11 that handles direct frame transmission without 
// contention. Manages inter-frame spacing (typically SIFS) before transmission, ensuring 
// proper timing between consecutive frames. Primarily used for control frames like ACK 
// and CTS, or for subsequent frames within a Transmission Opportunity (TXOP) where 
// medium access has already been secured.
//
// @see ~ITx
//
simple Tx like ITx
{
    parameters:
        string rxModule;
        @display("i=block/tx");
}
