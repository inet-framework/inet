//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORBLOCKACKAGREEMENTHANDLER_H
#define __INET_ORIGINATORBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements...
 * TODO OriginatorBlockAckAgreementProcedure?
 */
class INET_API OriginatorBlockAckAgreementHandler : public IOriginatorBlockAckAgreementHandler
{
  protected:
    std::map<std::pair<MacAddress, Tid>, OriginatorBlockAckAgreement *> blockAckAgreements;
    uint8_t nextDialogToken = 1;

  protected:
    virtual const Ptr<Ieee80211AddbaRequest> buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy);
    virtual uint8_t allocateDialogToken();
    virtual void createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest);
    virtual void updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp);
    virtual void terminateAgreement(MacAddress originatorAddr, Tid tid);
    virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
    virtual simtime_t computeEarliestExpirationTime();
    virtual simtime_t computeEarliestAddbaResponseDeadline() const;
    virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);
    virtual void scheduleAddbaResponseTimer(IBlockAckAgreementHandlerCallback *callback);

  public:
    virtual ~OriginatorBlockAckAgreementHandler();
    virtual void processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual void processTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) override;
    virtual void processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck, IBlockAckAgreementHandlerCallback *callback) override;
    virtual OriginatorBlockAckAgreement *processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy) override;
    virtual void processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba) override;
    virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;
    virtual void addbaResponseTimeoutExpired(IBlockAckAgreementHandlerCallback *callback) override;

    virtual OriginatorBlockAckAgreement *getAgreement(MacAddress receiverAddr, Tid tid) override;
    virtual bool isAddbaResponsePending(MacAddress receiverAddr, Tid tid) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
