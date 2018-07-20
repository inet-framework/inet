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

#ifndef __INET_RECIPIENTACKPROCEDURE_H
#define __INET_RECIPIENTACKPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRecipientAckProcedure.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.8 ACK procedure
 */
class INET_API RecipientAckProcedure : public IRecipientAckProcedure
{
    protected:
        int numReceivedAckableFrame = 0;
        int numSentAck = 0;

    protected:
        virtual const Ptr<Ieee80211AckFrame> buildAck(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const;

    public:
        virtual void processReceivedFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader, IRecipientAckPolicy *ackPolicy, IProcedureCallback *callback) override;
        virtual void processTransmittedAck(const Ptr<const Ieee80211AckFrame>& ack) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // __INET_RECIPIENTACKPROCEDURE_H
