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

#ifndef __INET_RECIPIENTBLOCKACKHANDLER_H
#define __INET_RECIPIENTBLOCKACKHANDLER_H

#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

class RecipientBlockAckAgreement;

/*
 * This class implements 9.21.3 Data and acknowledgment transfer using
 * immediate Block Ack policy and delayed Block Ack policy
 *
 * TODO: RecipientBlockAckAgreementProcedure ?
 */
class INET_API RecipientBlockAckAgreementHandler : public IRecipientBlockAckAgreementHandler
{
    protected:
        std::map<std::pair<MACAddress, Tid>, RecipientBlockAckAgreement *> blockAckAgreements;

    protected:
        virtual void terminateAgreement(MACAddress originatorAddr, Tid tid);
        virtual RecipientBlockAckAgreement* addAgreement(Ieee80211AddbaRequest *addbaReq);
        virtual void updateAgreement(Ieee80211AddbaResponse *frame);
        virtual Ieee80211AddbaResponse* buildAddbaResponse(Ieee80211AddbaRequest *frame, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy);
        virtual Ieee80211Delba* buildDelba(MACAddress receiverAddr, Tid tid, int reasonCode);
        virtual simtime_t computeEarliestExpirationTime();
        virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback* callback);

    public:
        virtual ~RecipientBlockAckAgreementHandler();
        virtual void processTransmittedAddbaResp(Ieee80211AddbaResponse *addbaResp, IBlockAckAgreementHandlerCallback *callback) override;
        virtual void processReceivedAddbaRequest(Ieee80211AddbaRequest *addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) override;
        virtual void processReceivedDelba(Ieee80211Delba *delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy) override;
        virtual void qosFrameReceived(Ieee80211DataFrame* qosFrame, IBlockAckAgreementHandlerCallback *callback) override;
        virtual void processTransmittedDelba(Ieee80211Delba *delba) override;
        virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;

        virtual RecipientBlockAckAgreement* getAgreement(Tid tid, MACAddress originatorAddr) override;
};

} // namespace ieee80211
} // namespace inet

#endif
