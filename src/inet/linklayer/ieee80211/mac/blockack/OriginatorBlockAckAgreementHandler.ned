//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

//
// Implements the IEEE 802.11e/n/ac originator Block Acknowledgment (Block ACK) agreement 
// handler that manages the lifecycle of Block ACK agreements from the originator's 
// perspective. This module works in conjunction with the OriginatorBlockAckAgreementPolicy 
// to establish, maintain, and terminate Block ACK agreements with recipient stations. 
// It generates ADDBA Request frames to initiate agreements, processes ADDBA Response 
// frames to finalize them, and handles DELBA frames for agreement termination. The handler 
// maintains the state of all active agreements, tracks sequence numbers for transmitted 
// frames, and coordinates with the Block ACK mechanism to ensure reliable frame delivery 
// in high-throughput scenarios.
//
// @see ~OriginatorBlockAckAgreementPolicy, ~RecipientBlockAckAgreementHandler
//
simple OriginatorBlockAckAgreementHandler
{
    parameters:
        string rateSelectionModule;

        @display("i=block/cogwheel");
}
