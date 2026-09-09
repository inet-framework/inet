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
    std::map<std::pair<MacAddress, Tid>, simtime_t> addbaRetryDeadlines;
    // A tagged local DELBA remains eligible after its agreement is removed
    // until the final fragment is acknowledged or terminally aborted.
    std::map<std::pair<MacAddress, Tid>, uint64_t> pendingTeardownTransactionIds;
    uint8_t nextDialogToken = 1;
    uint64_t nextTransactionId = 1;

  protected:
    virtual const Ptr<Ieee80211AddbaRequest> buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy);
    virtual uint8_t allocateDialogToken();
    virtual void createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, uint64_t transactionId);
    virtual void updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp);
    virtual OriginatorBlockAckAgreement *removeAgreement(MacAddress originatorAddr, Tid tid);
    virtual void terminateAgreement(MacAddress originatorAddr, Tid tid);
    virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
    virtual simtime_t computeEarliestExpirationTime();
    virtual simtime_t computeEarliestAddbaResponseDeadline() const;
    virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);
    virtual void scheduleAddbaResponseTimer(IBlockAckAgreementHandlerCallback *callback);
    virtual simtime_t getAddbaResponseTimeout(IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy) const;
    virtual void recordAddbaFailure(MacAddress receiverAddr, Tid tid, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy);

  public:
    virtual ~OriginatorBlockAckAgreementHandler();
    virtual void processTransmittedAddbaReq(Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual void processDroppedAddbaReq(Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual uint64_t processAcknowledgedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback) override;
    virtual void processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck, IBlockAckAgreementHandlerCallback *callback) override;
    virtual OriginatorBlockAckAgreementResponse processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual std::unique_ptr<OriginatorBlockAckAgreement> processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual std::unique_ptr<OriginatorBlockAckAgreement> processTransmittedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) override;
    virtual bool processAcknowledgedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) override;
    virtual bool processAbortedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) override;
    virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;
    virtual void addbaResponseTimeoutExpired(IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;

    virtual OriginatorBlockAckAgreement *getAgreement(MacAddress receiverAddr, Tid tid) override;
    virtual bool isAddbaResponsePending(MacAddress receiverAddr, Tid tid) const override;
    virtual bool isAddbaRequestPending(const Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq) const override;
    virtual bool isDelbaPending(const Packet *packet, const Ptr<const Ieee80211Delba>& delba) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
