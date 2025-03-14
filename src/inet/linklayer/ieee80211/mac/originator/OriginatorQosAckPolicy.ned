//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.originator;

import inet.linklayer.ieee80211.mac.contract.IOriginatorQosAckPolicy;


//
// Implements the IEEE 802.11e/n/ac originator Quality of Service (QoS) Acknowledgment 
// policy for QoS stations. This module extends the basic ACK policy with QoS-aware 
// features, including support for Block Acknowledgment (Block ACK) which allows 
// multiple frames to be acknowledged with a single response. The policy determines 
// when to use normal ACKs versus Block ACKs based on frame characteristics, traffic 
// categories, and configurable thresholds. It manages both immediate and delayed 
// Block ACK agreements and calculates appropriate timeout periods for different 
// acknowledgment types.
//
// @see ~IOriginatorQosAckPolicy, ~OriginatorAckPolicy, ~RecipientQosAckPolicy
//
simple OriginatorQosAckPolicy like IOriginatorQosAckPolicy
{
    parameters:
        string rateSelectionModule;

        int blockAckReqThreshold = default(5);
        int maxBlockAckPolicyFrameLength @unit(B) = default(1000B);
        double blockAckTimeout @unit(s) = default(-1s);
        double ackTimeout @unit(s) = default(-1s);

        @display("i=block/control");
}
