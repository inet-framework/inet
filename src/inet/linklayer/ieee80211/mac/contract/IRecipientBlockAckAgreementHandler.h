//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H
#define __INET_IRECIPIENTBLOCKACKAGREEMENTHANDLER_H

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/IBlockAckAgreementHandlerCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IProcedureCallback.h"
#include "inet/linklayer/ieee80211/mac/contract/IRecipientBlockAckAgreementPolicy.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientBlockAckAgreementHandler
{
  public:
    virtual ~IRecipientBlockAckAgreementHandler() {}

    // Return the earliest active absolute deadline, or SIMTIME_MAX if none exists.
    virtual simtime_t getEarliestExpirationTime() const = 0;

    virtual void processTransmittedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IProcedureCallback *callback) = 0;
    virtual void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba, IBlockAckAgreementHandlerCallback *callback) = 0;
    // Return false after retirement if this frame reaches an overdue agreement.
    virtual bool qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback) = 0;
    // Retire overdue state and complete deletion callbacks. Return timeout DELBA chunks.
    // The caller queues these chunks after both roles complete retirement.
    virtual std::vector<Ptr<Ieee80211Delba>> blockAckAgreementExpired(IBlockAckAgreementHandlerCallback *callback) = 0;

    virtual RecipientBlockAckAgreement *getAgreement(Tid tid, MacAddress originatorAddr) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

