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
        localCompressedBlockAckSupported = par("localCompressedBlockAckSupported");
        for (const auto& address : cStringTokenizer(par("compressedBlockAckPeerAddresses")).asVector())
            compressedBlockAckPeerAddresses.insert(MacAddress(address.c_str()));
        isDelayedBlockAckPolicySupported = par("delayedAckPolicySupported");
        isAMsduSupported = par("aMsduSupported");
        maximumAllowedBufferSize = par("maximumAllowedBufferSize");
        blockAckTimeoutValue = par("blockAckTimeoutValue");
    }
}

bool RecipientBlockAckAgreementPolicy::isPeerCompressedBlockAckSupported(const MacAddress& peerAddress) const
{
    return modeSet != nullptr && modeSet->isHtOperationSupported() && localCompressedBlockAckSupported && compressedBlockAckPeerAddresses.count(peerAddress) != 0;
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

