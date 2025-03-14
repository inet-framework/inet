//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

//
// Manages acknowledgment status tracking for IEEE 802.11 frames that require 
// acknowledgment. This module maintains a record of transmitted frames awaiting 
// acknowledgment and their status (acknowledged, unacknowledged, or expired). 
// It works with the recovery procedures to determine when retransmissions are 
// needed and helps implement the positive acknowledgment mechanism that ensures 
// reliable frame delivery in IEEE 802.11 networks. The handler tracks sequence 
// numbers and timing information to properly match incoming ACK frames with their 
// corresponding data frames.
//
// @see ~QosAckHandler, ~NonQosRecoveryProcedure, ~OriginatorAckPolicy
//
simple AckHandler
{
    parameters:
        @display("i=block/table2");
}
