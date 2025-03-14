//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

//
// Implements the IEEE 802.11e/n/ac recipient Block Acknowledgment (Block ACK) agreement 
// handler that manages the lifecycle of Block ACK agreements from the recipient's 
// perspective. This module works in conjunction with the RecipientBlockAckAgreementPolicy 
// to process, maintain, and terminate Block ACK agreements with originator stations. 
// It processes incoming ADDBA Request frames, generates ADDBA Response frames based on 
// policy decisions, and handles DELBA frames for agreement termination. The handler 
// maintains the state of all active agreements, tracks sequence numbers for received 
// frames, manages the scoreboard for tracking frame reception status, and generates 
// Block ACK responses when requested by the originator.
//
// @see ~RecipientBlockAckAgreementPolicy, ~OriginatorBlockAckAgreementHandler
//
simple RecipientBlockAckAgreementHandler
{
    parameters:
        string rateSelectionModule;

        @display("i=block/cogwheel");
}
