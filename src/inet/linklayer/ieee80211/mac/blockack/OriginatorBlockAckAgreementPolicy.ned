//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.blockack;

import inet.common.SimpleModule;
import inet.linklayer.ieee80211.mac.contract.IOriginatorBlockAckAgreementPolicy;

//
// Implements the default originator block ACK agreement policy
//
simple OriginatorBlockAckAgreementPolicy extends SimpleModule like IOriginatorBlockAckAgreementPolicy
{
    parameters:
        @class(OriginatorBlockAckAgreementPolicy);
        string originatorAckPolicyModule;

        bool delayedAckPolicySupported = default(false);
        bool aMsduSupported = default(true);
        int maximumAllowedBufferSize = default(64); // The ADDBA Request uses this advisory buffer size; the recipient response determines the accepted allocation.
        double blockAckTimeoutValue @unit(s) = default(0s); // Zero disables inactivity expiration.
        double addbaFailureTimeout @unit(s) = default(100ms); // Response wait after request ACK.
        @display("i=block/control");
}

