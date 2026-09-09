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
        int maximumAllowedBufferSize = default(64);
        double blockAckTimeoutValue @unit(s) = default(0s); // 0 means that it depends on the originator
        double addbaResponseTimeout @unit(s) = default(1s); // Model response-wait timeout after the ADDBA Request is transmitted
        double addbaRetryBackoff @unit(s) = default(1s); // Model retry suppression after a failed or discarded ADDBA transaction; not an IEEE 802.11 timer
        @display("i=block/control");
}
