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

#ifndef __IORIGINATORBLOCKACKAGREEMENTPOLICY_H
#define __IORIGINATORBLOCKACKAGREEMENTPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class OriginatorBlockAckAgreement;

class INET_API IOriginatorBlockAckAgreementPolicy
{
    public:
        virtual ~IOriginatorBlockAckAgreementPolicy() { }

        virtual bool isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) = 0;
        virtual bool isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement* agreement) = 0;
        virtual bool isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba) = 0;

        virtual bool isMsduSupported() const = 0;
        virtual simtime_t computeAddbaFailureTimeout() const = 0;
        virtual simtime_t getBlockAckTimeoutValue() const = 0;
        virtual bool isDelayedAckPolicySupported() const = 0;
        virtual int getMaximumAllowedBufferSize() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // ifndef __IORIGINATORBLOCKACKAGREEMENTPOLICY_H
