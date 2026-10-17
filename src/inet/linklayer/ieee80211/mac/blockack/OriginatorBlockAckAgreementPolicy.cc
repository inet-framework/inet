//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementPolicy.h"

#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorBlockAckAgreementPolicy);

void OriginatorBlockAckAgreementPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ackPolicy = check_and_cast<IOriginatorQoSAckPolicy *>(getModuleByPath(par("originatorAckPolicyModule")));
        delayedAckPolicySupported = par("delayedAckPolicySupported");
        aMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
        addbaFailureTimeout = par("addbaFailureTimeout");
        if (addbaFailureTimeout <= SIMTIME_ZERO || addbaFailureTimeout == SIMTIME_MAX)
            throw cRuntimeError("addbaFailureTimeout must be positive and finite");
        if (maximumAllowedBufferSize <= 0 || blockAckTimeoutValue < SIMTIME_ZERO)
            throw cRuntimeError("Invalid Block Ack buffer size or inactivity timeout");
        WATCH(blockAckReqThreshold);
    }
}

simtime_t OriginatorBlockAckAgreementPolicy::computeAddbaFailureTimeout() const
{
    return addbaFailureTimeout;
}

bool OriginatorBlockAckAgreementPolicy::isProfileAllowed(BlockAckVariant variant, bool localHt, bool peerHt) const
{
    if (variant == BlockAckVariant::COMPRESSED && (!localHt || !peerHt))
        EV_DEBUG << "Compressed Block Ack setup refused: " << (!localHt ? "local HT state is absent" : "peer HT state is absent") << "; DATA uses Normal Ack.\n";
    return variant == BlockAckVariant::BASIC || (localHt && peerHt);
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    return ackPolicy->isBlockAckPolicyEligibleFrame(packet, header);
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement *agreement)
{
    // IEEE Std 802.11-2024, 10.25.2 and Table 9-466: match the pending setup and its nonzero dialog token.
    // Immediate Block Ack and the 64-entry bound are INET profile limits for historical Basic compatibility.
    return agreement != nullptr && !agreement->getIsAddbaResponseReceived() &&
        addbaResp->getTransmitterAddress() == agreement->getReceiverAddr() &&
        addbaResp->getTid() == agreement->getTid() &&
        addbaResp->getDialogToken() > 0 && addbaResp->getDialogToken() <= 255 &&
        addbaResp->getDialogToken() == agreement->getDialogToken() &&
        addbaResp->getStatusCode() == SC_SUCCESSFUL && addbaResp->getBlockAckPolicy() &&
        addbaResp->getBufferSize() > 0 && addbaResp->getBufferSize() <= 64 &&
        addbaResp->getBlockAckTimeoutValue() >= SIMTIME_ZERO;
}

bool OriginatorBlockAckAgreementPolicy::isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba)
{
    return true;
}

} /* namespace ieee80211 */
} /* namespace inet */

