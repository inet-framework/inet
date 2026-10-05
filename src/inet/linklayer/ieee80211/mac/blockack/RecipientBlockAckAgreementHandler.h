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
    // IEEE Std 802.11-2024, Table 9-79.
    static constexpr int UNKNOWN_BA = 38;
    std::map<std::pair<MacAddress, Tid>, RecipientBlockAckAgreement *> blockAckAgreements;
    uint64_t lastAgreementId = 0;

  protected:
    virtual void terminateAgreement(MacAddress originatorAddr, Tid tid, IBlockAckAgreementHandlerCallback *callback);
    virtual RecipientBlockAckAgreement *addAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaReq);
    virtual void updateAgreement(const Ptr<const Ieee80211AddbaResponse>& addbaResponse);
    virtual const Ptr<Ieee80211AddbaResponse> buildAddbaResponse(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy);
    virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
    virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);

  public:
    virtual ~RecipientBlockAckAgreementHandler();
    [[nodiscard]] simtime_t getEarliestExpirationTime() const override;
    // A response belongs to the current setup only if its local identity and deadline match.
    [[nodiscard]] bool isAddbaResponseCurrent(const Ptr<const Ieee80211AddbaResponse>& addbaResp, uint64_t agreementId) const override;
    virtual void processTransmittedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, uint64_t agreementId, IBlockAckAgreementHandlerCallback *callback) override;
    virtual void processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) override;
    void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) override;
    virtual bool qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback, IProcedureCallback *procedureCallback) override;
    void processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba, IBlockAckAgreementHandlerCallback *callback) override;
    std::vector<Ptr<Ieee80211Delba>> blockAckAgreementExpired(IBlockAckAgreementHandlerCallback *callback) override;

    virtual RecipientBlockAckAgreement *getAgreement(Tid tid, MacAddress originatorAddr) override;
};

} // namespace ieee80211
} // namespace inet

#endif

