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
    class INET_API ICallback : public IProcedureCallback {
      public:
        // The previous agreement remains valid during this call. The new agreement is already current.
        virtual void recipientAgreementReplaced(RecipientBlockAckAgreement *previous, RecipientBlockAckAgreement *current) = 0;
    };

    virtual ~IRecipientBlockAckAgreementHandler() {}

    virtual void processTransmittedAddbaResp(const Ptr<const Ieee80211AddbaResponse>& addbaResp, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void processReceivedAddbaRequest(const Ptr<const Ieee80211AddbaRequest>& addbaRequest, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy, ICallback *callback) = 0;
    virtual void processReceivedDelba(const Ptr<const Ieee80211Delba>& delba, IRecipientBlockAckAgreementPolicy *blockAckAgreementPolicy) = 0;
    virtual void processTransmittedDelba(const Ptr<const Ieee80211Delba>& delba) = 0;
    // Report a terminal queue drop, acknowledgment, or retry exhaustion, not an individual attempt.
    virtual void processDelbaFrameFinished(const Packet *packet, IRecipientBlockAckAgreementPolicy *policy, ICallback *callback) = 0;
    virtual void qosFrameReceived(const Ptr<const Ieee80211DataHeader>& qosHeader, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void blockAckRequestReceived(const Ptr<const Ieee80211BasicBlockAckReq>& request, IBlockAckAgreementHandlerCallback *callback) = 0;
    virtual void blockAckAgreementExpired(IProcedureCallback *procedureCallback, IBlockAckAgreementHandlerCallback *agreementHandlerCallback) = 0;

    // Return the earliest active absolute deadline, or SIMTIME_MAX if none exists.
    virtual simtime_t computeEarliestExpirationTime() = 0;

    virtual RecipientBlockAckAgreement *getAgreement(Tid tid, MacAddress originatorAddr) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
