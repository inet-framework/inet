//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementPolicy.h"

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorBlockAckAgreementPolicy);

void OriginatorBlockAckAgreementPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        mib.reference(this, "mibModule", true);
        ackPolicy = check_and_cast<IOriginatorQoSAckPolicy *>(getModuleByPath(par("originatorAckPolicyModule")));
        delayedAckPolicySupported = par("delayedAckPolicySupported");
        aMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
        if (maximumAllowedBufferSize < 1 || maximumAllowedBufferSize > 64 || blockAckTimeoutValue < SIMTIME_ZERO)
            throw cRuntimeError("Unsupported BlockAck buffer size or timeout");
        // TODO addbaFailureTimeout = par("addbaFailureTimeout");
        WATCH(blockAckReqThreshold);
    }
}

simtime_t OriginatorBlockAckAgreementPolicy::computeAddbaFailureTimeout() const
{
    // TODO ADDBAFailureTimeout -- 6.3.29.2.2 Semantics of the service primitive
    throw cRuntimeError("Unimplemented");
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    // A complete HT data session needs an implemented A-MPDU path.
    return false;
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement *agreement)
{
    // IEEE Std 802.11-2024, 10.25.2: accept only the negotiated immediate HT profile.
    auto peer = mib->findPeerHtState(addbaResp->getTransmitterAddress());
    return agreement && agreement->getIsAddbaRequestSent() && !agreement->getIsAddbaResponseReceived() &&
            agreement->getDialogToken() == addbaResp->getDialogToken() && addbaResp->getStatusCode() == 0 &&
            addbaResp->getReceiverAddress() == mib->address && !addbaResp->isIncorrect() &&
            modeSet && modeSet->isHtOperationSupported() && mib->isHtOperationSupported() &&
            peer && peer->valid && peer->negotiatedCapabilities.localTxPeerRx.valid &&
            !delayedAckPolicySupported && addbaResp->getBlockAckPolicy() &&
            !agreement->getIsDelayedBlockAckPolicySupported() &&
            addbaResp->getBufferSize() >= 1 && addbaResp->getBufferSize() <= 64 &&
            addbaResp->getBufferSize() <= agreement->getBufferSize() &&
            addbaResp->getBlockAckTimeoutValue() >= SIMTIME_ZERO &&
            (!addbaResp->getAMsduSupported() || agreement->getIsAMsduSupported());
}

bool OriginatorBlockAckAgreementPolicy::isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba)
{
    return !delba->isIncorrect() && delba->getReceiverAddress() == mib->address &&
            !delba->getTransmitterAddress().isMulticast() && delba->getTid() >= 0 && delba->getTid() < 16;
}

} /* namespace ieee80211 */
} /* namespace inet */

