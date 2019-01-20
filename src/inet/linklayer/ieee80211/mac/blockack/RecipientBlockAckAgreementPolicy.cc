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
