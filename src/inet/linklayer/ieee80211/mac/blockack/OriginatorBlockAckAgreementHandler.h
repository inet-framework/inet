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
    std::map<AgreementId, Procedure> procedures;
    std::map<AgreementId, int> dialogTokens;

  protected:
    virtual const Ptr<Ieee80211AddbaRequest> buildAddbaRequest(MacAddress receiverAddr, Tid tid, SequenceNumberCyclic startingSequenceNumber, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy);
    virtual void createAgreement(const Ptr<const Ieee80211AddbaRequest>& addbaRequest);
    virtual void updateAgreement(OriginatorBlockAckAgreement *agreement, const Ptr<const Ieee80211AddbaResponse>& addbaResp);
    virtual void terminateAgreement(MacAddress originatorAddr, Tid tid);
    virtual const Ptr<Ieee80211Delba> buildDelba(MacAddress receiverAddr, Tid tid, int reasonCode);
    virtual void scheduleInactivityTimer(IBlockAckAgreementHandlerCallback *callback);

  public:
    virtual simtime_t computeEarliestExpirationTime() override;
    const std::map<AgreementId, OriginatorBlockAckAgreement *>& getAgreements() const override { return blockAckAgreements; }
    const std::map<AgreementId, Procedure>& getProcedures() const override { return procedures; }
    virtual void noteEligibleDataTransmission(MacAddress peer, Tid tid) override;
    virtual void beginSetupIntent(MacAddress peer, Tid tid) override;
    virtual void beginTeardown(MacAddress peer, Tid tid) override;
    virtual void failTeardown(MacAddress peer, Tid tid) override;
    virtual void finishProcedure(MacAddress peer, Tid tid) override;
    virtual void markRequestAttempted(MacAddress peer, Tid tid) override;
    virtual void acknowledgeRequest(MacAddress peer, Tid tid, simtime_t timeout) override;
    OriginatorBlockAckAgreement *detachAgreement(MacAddress peer, Tid tid) override;
    std::vector<OriginatorBlockAckAgreement *> detachPeer(MacAddress peer) override;
    void startSetup(MacAddress peer, Tid tid, SequenceNumberCyclic sequence, IOriginatorBlockAckAgreementPolicy *policy, IProcedureCallback *callback, bool aMsdu) override;
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

