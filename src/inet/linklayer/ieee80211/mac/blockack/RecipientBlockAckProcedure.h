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

#ifndef __INET_RECIPIENTBLOCKACKPROCEDURE_H
#define __INET_RECIPIENTBLOCKACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckProcedure.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.9 BlockAck procedure
 */
class INET_API RecipientBlockAckProcedure : public IRecipientBlockAckProcedure
{
    protected:
        int numReceivedBlockAckReq = 0;
        int numSentBlockAck = 0;

    protected:
        virtual const Ptr<Ieee80211BlockAck> buildBlockAck(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement);

    public:
        virtual void processReceivedBlockAckReq(Packet *blockAckPacket, const Ptr<const Ieee80211BlockAckReq>& blockAckReq, IRecipientQosAckPolicy *ackPolicy, IRecipientBlockAckAgreementHandler* blockAckAgreementHandler, IProcedureCallback *callback) override;
        virtual void processTransmittedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTBLOCKACKPROCEDURE_H
