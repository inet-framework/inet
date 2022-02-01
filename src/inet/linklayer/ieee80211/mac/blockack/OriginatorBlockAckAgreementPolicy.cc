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
        ackPolicy = check_and_cast<IOriginatorQoSAckPolicy *>(getModuleByPath(par("originatorAckPolicyModule")));
        delayedAckPolicySupported = par("delayedAckPolicySupported");
        aMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
        // TODO addbaFailureTimeout = par("addbaFailureTimeout");
    }
}

simtime_t OriginatorBlockAckAgreementPolicy::computeAddbaFailureTimeout() const
{
    // TODO ADDBAFailureTimeout -- 6.3.29.2.2 Semantics of the service primitive
    throw cRuntimeError("Unimplemented");
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    return ackPolicy->isBlockAckPolicyEligibleFrame(packet, header);
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement *agreement)
{
    ASSERT(agreement);
    return true;
}

bool OriginatorBlockAckAgreementPolicy::isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba)
{
    return true;
}

} /* namespace ieee80211 */
} /* namespace inet */

