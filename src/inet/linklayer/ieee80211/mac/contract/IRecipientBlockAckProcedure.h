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

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientQosAckPolicy.h"

#ifndef __INET_IRECIPIENTBLOCKACKPROCEDURE_H
#define __INET_IRECIPIENTBLOCKACKPROCEDURE_H

namespace inet {
namespace ieee80211 {

class INET_API IRecipientBlockAckProcedure
{
    public:
        virtual ~IRecipientBlockAckProcedure() { }

        virtual void processReceivedBlockAckReq(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq, IRecipientQosAckPolicy *ackPolicy, IRecipientBlockAckAgreementHandler* blockAckAgreementHandler, IProcedureCallback *callback) = 0;
        virtual void processTransmittedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // ifndef __INET_IRECIPIENTBLOCKACKPROCEDURE_H
