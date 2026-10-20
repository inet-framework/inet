//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(RecipientBlockAckAgreementPolicy);

void RecipientBlockAckAgreementPolicy::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        isDelayedBlockAckPolicySupported = par("delayedAckPolicySupported");
        isAMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
        if (maximumAllowedBufferSize <= 0 || blockAckTimeoutValue < SIMTIME_ZERO)
            throw cRuntimeError("Invalid Block Ack buffer size or inactivity timeout");
    }
}

bool RecipientBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    // IEEE Std 802.11-2024, 10.25.2 and Table 9-466: validate ordinary setup; the requested buffer size is advisory.
    // INET supports immediate Block Ack for historical Basic compatibility and selects at most 64 entries in the response.
    return !addbaReq->getTransmitterAddress().isUnspecified() &&
        !addbaReq->getTransmitterAddress().isMulticast() &&
        addbaReq->getTid() >= 0 && addbaReq->getTid() <= 15 &&
        addbaReq->getDialogToken() > 0 && addbaReq->getDialogToken() <= 255 &&
        addbaReq->getBlockAckPolicy() && addbaReq->getBufferSize() >= 0 &&
        addbaReq->getBufferSize() <= 1023 && addbaReq->getStartingSequenceNumber().isSpecified() &&
        addbaReq->getBlockAckTimeoutValue() >= SIMTIME_ZERO;
}

bool RecipientBlockAckAgreementPolicy::isProfileAllowed(BlockAckVariant variant, bool localHt, bool peerHt) const
{
    return variant == BlockAckVariant::BASIC || (localHt && peerHt);
}

bool RecipientBlockAckAgreementPolicy::isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba)
{
    return true;
}

} /* namespace ieee80211 */
} /* namespace inet */

