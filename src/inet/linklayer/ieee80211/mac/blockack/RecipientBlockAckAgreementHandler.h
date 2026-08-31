//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_RECIPIENTBLOCKACKAGREEMENTHANDLER_H
#define __INET_RECIPIENTBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementHandler.h"

namespace inet {
namespace ieee80211 {

class RecipientBlockAckAgreement;

/*
 * This class implements 9.21.3 Data and acknowledgment transfer using
 * immediate Block Ack policy and delayed Block Ack policy
 *
 * TODO RecipientBlockAckAgreementProcedure ?
 */
class INET_API RecipientBlockAckAgreementHandler : public IRecipientBlockAckAgreementHandler
{
  protected:
    std::map<std::pair<MacAddress, Tid>, RecipientBlockAckAgreement *> blockAckAgreements;
    std::map<std::pair<MacAddress, Tid>, Ptr<const Ieee80211AddbaResponse>> lastAddbaResponses;
    // A tagged local DELBA remains eligible after its agreement is removed
    // until the final fragment is acknowledged or terminally aborted.
    std::map<std::pair<MacAddress, Tid>, uint64_t> pendingTeardownGenerationIds;
    uint64_t nextAgreementGenerationId = 1;

  protected:
    virtual RecipientBlockAckAgreement *removeAgreement(MacAddress originatorAddr, Tid tid);
    virtual const Ptr<Ieee80211AddbaResponse> buildAddbaResponse(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, bool accepted);
    virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
    virtual uint64_t allocateAgreementGenerationId();
    virtual simtime_t computeEarliestExpirationTime();
    virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);

  public:
    virtual ~RecipientBlockAckAgreementHandler();
    virtual RecipientBlockAckAgreement *processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;
    virtual void processDuplicateAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IProcedureCallback *procedureCallback) override;
    virtual std::unique_ptr<RecipientBlockAckAgreement> processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback = nullptr) override;
    virtual void qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback) override;
    virtual std::unique_ptr<RecipientBlockAckAgreement> processTransmittedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback = nullptr) override;
    virtual bool processAcknowledgedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) override;
    virtual RecipientBlockAckAgreementAbortResult processAbortedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) override;
    virtual void blockAckReqReceived(const Ptr<const Ieee80211BasicBlockAckReq>& blockAckReq, IBlockAckAgreementHandlerCallback *callback) override;
    virtual bool blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;

    virtual RecipientBlockAckAgreement *getAgreement(Tid tid, MacAddress originatorAddr) override;
    virtual RecipientBlockAckAgreement *getActiveAgreement(Tid tid, MacAddress originatorAddr) override;
    virtual uint64_t getPendingTeardownGenerationId(Tid tid, MacAddress originatorAddr) const override;
    virtual bool isDelbaPending(const Packet *packet, const Ptr<const Ieee80211Delba>& delba) const override;
};

} // namespace ieee80211
} // namespace inet

#endif
