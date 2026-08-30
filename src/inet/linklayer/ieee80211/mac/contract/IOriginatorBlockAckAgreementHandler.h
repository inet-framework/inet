//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORBLOCKACKAGREEMENTHANDLER_H
#define __INET_IORIGINATORBLOCKACKAGREEMENTHANDLER_H

#include <memory>

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorBlockAckAgreementPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"

namespace inet {
namespace ieee80211 {

struct INET_API OriginatorBlockAckAgreementResponse
{
    // Borrowed from the handler; valid while the established agreement remains installed.
    OriginatorBlockAckAgreement *establishedAgreement = nullptr;
    // Owns the agreement that was established and immediately terminated after a local veto.
    std::unique_ptr<OriginatorBlockAckAgreement> terminatedAgreement;
    Ptr<const Ieee80211Delba> teardownDelba;
    uint64_t teardownTransactionId = 0;
};

struct INET_API OriginatorBlockAckAgreementAbortResult
{
    bool handled = false;
    std::unique_ptr<OriginatorBlockAckAgreement> terminatedAgreement;

    explicit operator bool() const { return handled; }
};

class INET_API IOriginatorBlockAckAgreementHandler
{
  public:
    virtual ~IOriginatorBlockAckAgreementHandler() {}

    virtual void processReceivedBlockAck(const Ptr<const Ieee80211BlockAck>& blockAck, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processTransmittedAddbaReq(Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processDroppedAddbaReq(Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
    // Returns the transaction identity of an obsolete teardown whose packets
    // must be cancelled by the caller, or 0 when there is none.
    virtual uint64_t processAcknowledgedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *procedureCallback) = 0;
    virtual OriginatorBlockAckAgreementResponse processReceivedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual std::unique_ptr<OriginatorBlockAckAgreement> processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual std::unique_ptr<OriginatorBlockAckAgreement> processTransmittedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) = 0;
    // Returns true when the packet completed or aborted its tagged teardown
    // transaction and sibling packets were cancelled through the callback.
    virtual bool processAcknowledgedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual OriginatorBlockAckAgreementAbortResult processAbortedDelba(Packet *packet, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) = 0;
    virtual void addbaResponseTimeoutExpired(IOriginatorBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;

    virtual OriginatorBlockAckAgreement *getAgreement(MacAddress receiverAddr, Tid tid) = 0;
    virtual bool isAddbaResponsePending(MacAddress receiverAddr, Tid tid) const = 0;
    virtual bool isAddbaRequestPending(const Packet *packet, const Ptr<const Ieee80211AddbaRequest>& addbaReq) const = 0;
    virtual bool isDelbaPending(const Packet *packet, const Ptr<const Ieee80211Delba>& delba) const { return true; }
};

} // namespace ieee80211
} // namespace inet

#endif
