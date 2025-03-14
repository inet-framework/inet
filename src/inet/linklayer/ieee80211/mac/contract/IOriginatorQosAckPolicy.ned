//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11e/n/ac originator Quality of Service (QoS) Acknowledgment 
// policies that extend the basic ACK mechanism with QoS-aware features. These policies 
// determine the acknowledgment strategy for QoS data frames, including support for 
// Block Acknowledgment (Block ACK) which allows multiple frames to be acknowledged 
// with a single response. QoS ACK policies consider traffic categories, access 
// categories, and QoS requirements when deciding on acknowledgment methods and 
// timeout periods, enabling more efficient medium usage for different traffic types.
//
// @see ~OriginatorQosAckPolicy, ~IOriginatorAckPolicy, ~IRecipientQosAckPolicy
//
moduleinterface IOriginatorQosAckPolicy
{
    parameters:
        string rateSelectionModule;
        @display("i=block/control");
}
