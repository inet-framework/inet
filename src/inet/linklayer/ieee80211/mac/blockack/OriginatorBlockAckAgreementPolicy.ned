//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

import inet.linklayer.ieee80211.mac.contract.IOriginatorBlockAckAgreementPolicy;


//
// Implements the standard IEEE 802.11e/n/ac originator Block Acknowledgment (Block ACK) 
// agreement policy. This module determines when to establish, maintain, and terminate 
// Block ACK agreements with recipient stations. It supports both immediate and delayed 
// Block ACK mechanisms, configurable buffer sizes, and timeout values. The policy 
// evaluates traffic conditions to decide when Block ACK would be beneficial, initiates 
// the agreement setup through ADDBA Request/Response frames, and monitors the agreement 
// for potential updates or termination. It works in conjunction with the originator ACK 
// policy to provide efficient frame acknowledgment in high-throughput scenarios.
//
// @see ~IOriginatorBlockAckAgreementPolicy, ~RecipientBlockAckAgreementPolicy, ~OriginatorBlockAckAgreementHandler
//
simple OriginatorBlockAckAgreementPolicy like IOriginatorBlockAckAgreementPolicy
{
    parameters:
        string originatorAckPolicyModule;

        bool delayedAckPolicySupported = default(false);
        bool aMsduSupported = default(true);
        int maximumAllowedBufferSize = default(64);
        double blockAckTimeoutValue @unit(s) = default(0s); // 0 means that it depends on the originator
        @display("i=block/control");
}
