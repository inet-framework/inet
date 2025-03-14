//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11e/n/ac recipient Quality of Service (QoS) Acknowledgment 
// policies that extend the basic ACK mechanism with QoS-aware features. These policies 
// determine how to respond to received QoS data frames, including support for Block 
// Acknowledgment (Block ACK) which allows multiple frames to be acknowledged with 
// a single response. QoS ACK policies consider traffic categories, access categories, 
// and QoS requirements when deciding on acknowledgment methods, enabling more 
// efficient medium usage for different traffic types. They handle both immediate 
// and delayed Block ACK agreements and manage Block ACK request/response exchanges.
//
// @see ~RecipientQosAckPolicy, ~IRecipientAckPolicy, ~IOriginatorQosAckPolicy
//
moduleinterface IRecipientQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
