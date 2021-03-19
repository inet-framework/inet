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

#ifndef __INET_ORIGINATORBLOCKACKAGREEMENTHANDLER_H
#define __INET_ORIGINATORBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements...
 * TODO: OriginatorBlockAckAgreementProcedure?
 */
class INET_API OriginatorBlockAckAgreementHandler : public IOriginatorBlockAckAgreementHandler
{
    protected:
        std::map<std::pair<MacAddress, Tid>, OriginatorBlockAckAgreement *> blockAckAgreements;

    protected:
        virtual const Ptr<Ieee80211AddbaRequest> buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy* blockAckAgreementPolicy);
        virtual void createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest);
        virtual void updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp);
        virtual void terminateAgreement(MacAddress originatorAddr, Tid tid);
        virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
        virtual simtime_t computeEarliestExpirationTime();
        virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);

    public:
        virtual ~OriginatorBlockAckAgreementHandler();
        virtual void processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq) override;
        virtual void processTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) override;
        virtual void processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck, IBlockAckAgreementHandlerCallback *callback) override;
        virtual void processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
        virtual void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy) override;
        virtual void processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba) override;
        virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;

        virtual OriginatorBlockAckAgreement *getAgreement(MacAddress receiverAddr, Tid tid) override;
};

} // namespace ieee80211
} // namespace inet

#endif
