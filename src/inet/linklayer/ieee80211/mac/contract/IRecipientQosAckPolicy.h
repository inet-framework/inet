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

#ifndef __INET_IRECIPIENTQOSACKPOLICY_H
#define __INET_IRECIPIENTQOSACKPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class RecipientBlockAckAgreement;

class INET_API IRecipientQosAckPolicy
{
    public:
        virtual ~IRecipientQosAckPolicy() { }

        virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;
        virtual bool isBlockAckNeeded(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement) const = 0;

        virtual simtime_t computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;
        virtual simtime_t computeBasicBlockAckDurationField(Packet *packet, const Ptr<const Ieee80211BasicBlockAckReq>& basicBlockAckReq) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // ifndef __INET_IRECIPIENTQOSACKPOLICY_H
