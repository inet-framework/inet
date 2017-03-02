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

#ifndef __INET_IORIGINATORQOSACKPOLICY_H
#define __INET_IORIGINATORQOSACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorQoSAckPolicy
{
    public:
        virtual ~IOriginatorQoSAckPolicy() { };

        virtual bool isAckNeeded(Ieee80211ManagementFrame *frame) const = 0;
        virtual AckPolicy computeAckPolicy(Ieee80211DataFrame* frame, OriginatorBlockAckAgreement *agreement) const = 0;
        virtual bool isBlockAckReqNeeded(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure) const = 0;
        virtual bool isBlockAckPolicyEligibleFrame(Ieee80211DataFrame* frame) const = 0;
        virtual std::tuple<MACAddress, SequenceNumber, Tid> computeBlockAckReqParameters(InProgressFrames *inProgressFrames, TxopProcedure* txopProcedure) const = 0;

        virtual simtime_t getAckTimeout(Ieee80211DataOrMgmtFrame *dataOrMgmtFrame) const = 0;
        virtual simtime_t getBlockAckTimeout(Ieee80211BlockAckReq *blockAckReq) const = 0;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IORIGINATORQOSACKPOLICY_H
