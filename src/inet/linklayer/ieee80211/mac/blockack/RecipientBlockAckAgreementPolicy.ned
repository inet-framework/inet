//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

import inet.linklayer.ieee80211.mac.contract.IRecipientBlockAckAgreementPolicy;


//
// Implements the standard IEEE 802.11e/n/ac recipient Block Acknowledgment (Block ACK) 
// agreement policy. This module determines how to respond to Block ACK agreement requests 
// from originator stations. It evaluates incoming ADDBA Request frames and decides whether 
// to accept or reject them based on available resources and capabilities. The policy 
// supports both immediate and delayed Block ACK mechanisms, configurable buffer sizes, 
// and timeout values. When accepting an agreement, it allocates the necessary resources 
// to track received frames and generate Block ACK responses. It also monitors existing 
// agreements for inactivity or errors to determine when they should be terminated.
//
// @see ~IRecipientBlockAckAgreementPolicy, ~OriginatorBlockAckAgreementPolicy, ~RecipientBlockAckAgreementHandler
//
simple RecipientBlockAckAgreementPolicy like IRecipientBlockAckAgreementPolicy
{
    parameters:
        bool delayedAckPolicySupported = default(false);
        bool aMsduSupported = default(true);
        int maximumAllowedBufferSize = default(64);
        double blockAckTimeoutValue @unit(s) = default(0s); // 0 means that it depends on the originator
        @display("i=block/control");
}
