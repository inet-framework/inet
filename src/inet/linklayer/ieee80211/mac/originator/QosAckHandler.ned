//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

//
// Extends the basic acknowledgment handling functionality for IEEE 802.11e/n/ac QoS 
// stations. This module manages acknowledgment tracking for QoS frames, supporting 
// both immediate acknowledgments and block acknowledgments. It maintains per-TID 
// (Traffic ID) sequence number tracking and handles the more complex acknowledgment 
// mechanisms required by QoS service classes. The module works with QoS recovery 
// procedures to manage retransmissions based on traffic priority levels and supports 
// the enhanced distributed channel access (EDCA) mechanism for prioritized QoS traffic.
//
// @see ~AckHandler, ~QosRecoveryProcedure, ~OriginatorQosAckPolicy
//
simple QosAckHandler
{
    parameters:
        @display("i=block/table2");
}
