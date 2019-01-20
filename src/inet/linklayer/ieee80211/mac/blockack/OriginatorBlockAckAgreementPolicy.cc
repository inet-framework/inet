//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
// 

#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

Define_Module(OriginatorBlockAckAgreementPolicy);

void OriginatorBlockAckAgreementPolicy::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ackPolicy = check_and_cast<IOriginatorQoSAckPolicy*>(getModuleByPath(par("originatorAckPolicyModule")));
        delayedAckPolicySupported = par("delayedAckPolicySupported");
        aMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
        // TODO: addbaFailureTimeout = par("addbaFailureTimeout");
    }
}

simtime_t OriginatorBlockAckAgreementPolicy::computeAddbaFailureTimeout() const
{
    // TODO: ADDBAFailureTimeout -- 6.3.29.2.2 Semantics of the service primitive
    throw cRuntimeError("Unimplemented");
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header)
{
    return ackPolicy->isBlockAckPolicyEligibleFrame(packet, header);
}

bool OriginatorBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement* agreement)
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
