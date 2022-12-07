//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

import inet.linklayer.ieee80211.mac.contract.IRecipientBlockAckAgreementPolicy;


//
// Implements the default recipient block ACK agreement policy.
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

