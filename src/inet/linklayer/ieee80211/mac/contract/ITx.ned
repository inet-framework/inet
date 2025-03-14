//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11 transmitter components that handle direct frame transmission 
// without contention. Defines functionality for transmitting frames after a specified 
// inter-frame space (typically SIFS). Used for time-critical control frames (ACK, CTS) 
// and subsequent data frames within a Transmission Opportunity (TXOP) where the medium 
// has already been reserved, eliminating the need for additional contention procedures.
//
// @see ~Tx
//
moduleinterface ITx
{
    parameters:
        @display("i=block/tx");
}
