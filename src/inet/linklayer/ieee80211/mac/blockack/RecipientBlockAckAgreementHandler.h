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

  protected:
    virtual RecipientBlockAckAgreement *removeAgreement(MacAddress originatorAddr, Tid tid);
    virtual const Ptr<Ieee80211AddbaResponse> buildAddbaResponse(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, bool accepted);
    virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
    virtual simtime_t computeEarliestExpirationTime();
    virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);

  public:
    virtual ~RecipientBlockAckAgreementHandler();
    virtual RecipientBlockAckAgreement *processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;
    virtual void processDuplicateAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IProcedureCallback *procedureCallback) override;
    virtual std::unique_ptr<RecipientBlockAckAgreement> processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy) override;
    virtual void qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback) override;
    virtual std::unique_ptr<RecipientBlockAckAgreement> processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba) override;
    virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) override;

    virtual RecipientBlockAckAgreement *getAgreement(Tid tid, MacAddress originatorAddr) override;
};

} // namespace ieee80211
} // namespace inet

#endif
