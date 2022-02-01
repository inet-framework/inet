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
    }
}

bool RecipientBlockAckAgreementPolicy::isAddbaReqAccepted(const Ptr<const Ieee80211AddbaRequest>& addbaReq)
{
    return true;
}

bool RecipientBlockAckAgreementPolicy::isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba)
{
    return true;
}

} /* namespace ieee80211 */
} /* namespace inet */

