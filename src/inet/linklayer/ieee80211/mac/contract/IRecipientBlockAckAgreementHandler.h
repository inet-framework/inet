//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H
#define __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H

#include <memory>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"

namespace inet {

class Packet;

namespace ieee80211 {

struct INET_API RecipientBlockAckAgreementAbortResult
{
    bool handled = false;
    std::unique_ptr<RecipientBlockAckAgreement> terminatedAgreement;

    explicit operator bool() const { return handled; }
};

class INET_API IRecipientBlockAckAgreementHandler
{
  public:
    virtual ~IRecipientBlockAckAgreementHandler() {}

    virtual RecipientBlockAckAgreement *processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) = 0;
    virtual void processDuplicateAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IProcedureCallback *procedureCallback) = 0;
    virtual std::unique_ptr<RecipientBlockAckAgreement> processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback = nullptr) = 0;
    virtual std::unique_ptr<RecipientBlockAckAgreement> processTransmittedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback = nullptr) = 0;
    virtual bool processAcknowledgedDelba(Packet *, IBlockAckAgreementHandlerCallback *) { return false; }
    virtual RecipientBlockAckAgreementAbortResult processAbortedDelba(Packet *, IBlockAckAgreementHandlerCallback *) { return {}; }
    virtual void qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void blockAckReqReceived(const Ptr<const Ieee80211BasicBlockAckReq>& blockAckReq, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual bool blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) = 0;

    virtual RecipientBlockAckAgreement *getAgreement(Tid tid, MacAddress originatorAddr) = 0;
    // Returns the installed agreement only while it can be used by the data
    // plane. Lifecycle code must use getAgreement() to retain generation-safe
    // teardown state after inactivity expiry.
    virtual RecipientBlockAckAgreement *getActiveAgreement(Tid tid, MacAddress originatorAddr) = 0;
    virtual uint64_t getPendingTeardownGenerationId(Tid, MacAddress) const { return 0; }
    virtual bool isDelbaPending(const Packet *packet, const Ptr<const Ieee80211Delba>& delba) const { return true; }
};

} // namespace ieee80211
} // namespace inet

#endif
