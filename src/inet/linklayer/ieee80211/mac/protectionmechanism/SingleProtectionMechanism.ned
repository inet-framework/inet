//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.protectionmechanism;

//
// Implements the Duration/ID field calculation for IEEE 802.11e/n/ac frames transmitted by QoS stations 
// under Enhanced Distributed Channel Access (EDCA). This module computes duration values according to 
// section 8.2.5 of the IEEE 802.11 standard, handling the more complex QoS cases including Block Ack 
// frames and different acknowledgment policies. It supports various frame types (RTS, CTS, BlockAckReq, 
// BlockAck, data, and management frames) and works with TXOP procedures to determine if a frame is the 
// final fragment of a transmission opportunity. The module integrates with QoS rate selection to 
// calculate accurate transmission times for frames at different data rates.
//
// @see ~OriginatorProtectionMechanism
//
simple SingleProtectionMechanism
{
    parameters:
        string rateSelectionModule;
        @display("i=block/encrypt");
}
