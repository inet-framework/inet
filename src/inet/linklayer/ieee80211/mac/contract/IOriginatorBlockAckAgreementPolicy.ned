//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11e/n/ac originator Block Acknowledgment (Block ACK) agreement 
// policies that determine when and how to establish, maintain, and terminate Block ACK 
// agreements with recipient stations. Block ACK is an advanced acknowledgment mechanism 
// that allows multiple frames to be acknowledged with a single response, improving 
// efficiency in high-throughput environments. The originator policy decides when to 
// initiate a Block ACK agreement, what parameters to use (such as buffer size and 
// timeout values), and when to update or delete existing agreements based on traffic 
// conditions and QoS requirements.
//
// @see ~OriginatorBlockAckAgreementPolicy, ~IRecipientBlockAckAgreementPolicy, ~OriginatorBlockAckAgreementHandler
//
moduleinterface IOriginatorBlockAckAgreementPolicy
{
    parameters:
        string originatorAckPolicyModule;
        @display("i=block/control");
}
