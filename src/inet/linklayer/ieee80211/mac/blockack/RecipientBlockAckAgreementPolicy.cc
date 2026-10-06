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
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        isDelayedBlockAckPolicySupported = par("delayedAckPolicySupported");
        isAMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
        if (maximumAllowedBufferSize < 1 || maximumAllowedBufferSize > 64 || blockAckTimeoutValue < SIMTIME_ZERO)
            throw cRuntimeError("Unsupported BlockAck buffer size or timeout");
    }
}

bool RecipientBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    // IEEE Std 802.11-2024, 10.25.2: absent peer evidence does not permit HT.
    auto peer = mib->findPeerHtState(addbaReq->getTransmitterAddress());
    return modeSet && modeSet->isHtOperationSupported() && mib->isHtOperationSupported() &&
            peer && peer->valid && peer->negotiatedCapabilities.localRxPeerTx.valid &&
            addbaReq->getReceiverAddress() == mib->address && !addbaReq->isIncorrect() &&
            !isDelayedBlockAckPolicySupported && addbaReq->getBlockAckPolicy() &&
            addbaReq->getTid() >= 0 && addbaReq->getTid() < 16 &&
            addbaReq->getBufferSize() >= 1 && addbaReq->getBlockAckTimeoutValue() >= SIMTIME_ZERO &&
            addbaReq->get_fragmentNumber() == 0;
}

bool RecipientBlockAckAgreementPolicy::isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba)
{
    return !delba->isIncorrect() && delba->getReceiverAddress() == mib->address &&
            !delba->getTransmitterAddress().isMulticast() && delba->getTid() >= 0 && delba->getTid() < 16;
}

} /* namespace ieee80211 */
} /* namespace inet */

