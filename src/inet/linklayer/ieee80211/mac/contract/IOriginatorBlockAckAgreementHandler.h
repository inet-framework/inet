//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORBLOCKACKAGREEMENTHANDLER_H
#define __INET_IORIGINATORBLOCKACKAGREEMENTHANDLER_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorBlockAckAgreementHandler
{
  public:
    typedef std::pair<MacAddress, Tid> AgreementId;
    enum class ProcedureState { SETUP_INTENT, REQUEST_PENDING, TEARDOWN_PENDING, RECOVERY_NEEDED, WAITING_FOR_SUCCESSOR };
    struct Procedure {
        ProcedureState state = ProcedureState::SETUP_INTENT;
        bool requestAttempted = false;
        simtime_t responseDeadline = SIMTIME_MAX;
    };
    virtual ~IOriginatorBlockAckAgreementHandler() {}
    virtual const std::map<AgreementId, OriginatorBlockAckAgreement *>& getAgreements() const = 0;
    virtual const std::map<AgreementId, Procedure>& getProcedures() const = 0;
    virtual simtime_t computeEarliestExpirationTime() = 0;
    virtual OriginatorBlockAckAgreement *detachAgreement(MacAddress peer, Tid tid) = 0;
    virtual std::vector<OriginatorBlockAckAgreement *> detachPeer(MacAddress peer) = 0;
    virtual void noteEligibleDataTransmission(MacAddress peer, Tid tid) = 0;
    virtual void beginSetupIntent(MacAddress peer, Tid tid) = 0;
    virtual void beginTeardown(MacAddress peer, Tid tid) = 0;
    virtual void failTeardown(MacAddress peer, Tid tid) = 0;
    virtual void finishProcedure(MacAddress peer, Tid tid) = 0;
    virtual void markRequestAttempted(MacAddress peer, Tid tid) = 0;
    virtual void acknowledgeRequest(MacAddress peer, Tid tid, simtime_t timeout) = 0;
    virtual void startSetup(MacAddress peer, Tid tid, SequenceNumberCyclic sequence, IOriginatorBlockAckAgreementPolicy *policy, IProcedureCallback *callback, bool aMsdu) = 0;

    virtual void processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processTransmittedAddbaReq(const Ptr<const Ieee80211AddbaRequest>& addbaReq) = 0;
    virtual void processTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) = 0;
    virtual void processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy) = 0;
    virtual void processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba) = 0;
    virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) = 0;

    virtual OriginatorBlockAckAgreement *getAgreement(MacAddress receiverAddr, Tid tid) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

