//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.contract;

//
// Interface for IEEE 802.11e/n/ac recipient Block Acknowledgment (Block ACK) agreement 
// policies that determine how to respond to Block ACK agreement requests from originator 
// stations. Block ACK is an advanced acknowledgment mechanism that allows multiple frames 
// to be acknowledged with a single response, improving efficiency in high-throughput 
// environments. The recipient policy decides whether to accept or reject incoming Block 
// ACK agreement requests, what parameters to negotiate (such as buffer size and timeout 
// values), and when to terminate existing agreements based on resource availability and 
// QoS requirements.
//
// @see ~RecipientBlockAckAgreementPolicy, ~IOriginatorBlockAckAgreementPolicy, ~RecipientBlockAckAgreementHandler
//
moduleinterface IRecipientBlockAckAgreementPolicy
{
    parameters:
        @display("i=block/control");
}
