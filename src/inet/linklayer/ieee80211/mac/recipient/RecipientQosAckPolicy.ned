//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.linklayer.ieee80211.mac.contract.IRecipientQosAckPolicy;


//
// Implements the IEEE 802.11e/n/ac recipient Quality of Service (QoS) Acknowledgment 
// policy for QoS stations. This module extends the basic ACK policy with QoS-aware 
// features, including support for Block Acknowledgment (Block ACK) which allows 
// multiple frames to be acknowledged with a single response. The policy determines 
// the appropriate acknowledgment method for received QoS data frames based on frame 
// characteristics, traffic categories, and existing Block ACK agreements. It handles 
// both immediate and delayed Block ACK mechanisms, processes Block ACK requests, 
// and generates Block ACK responses with proper bitmap information.
//
// @see ~IRecipientQosAckPolicy, ~RecipientAckPolicy, ~OriginatorQosAckPolicy
//
simple RecipientQosAckPolicy like IRecipientQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
