//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORQOSACKPOLICY_H
#define __INET_IORIGINATORQOSACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h"
#include "inet/linklayer/ieee80211/mac/originator/TxopProcedure.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"

namespace inet {
namespace ieee80211 {

class INET_API IOriginatorQoSAckPolicy
{
  public:
    virtual ~IOriginatorQoSAckPolicy() {}

    virtual bool isAckNeeded(const Ptr<const Ieee80211MgmtHeader>& header) const = 0;
    virtual AckPolicy computeAckPolicy(Packet *packet, const Ptr<const Ieee80211DataHeader>& header, OriginatorBlockAckAgreement *agreement) const = 0;
    virtual bool isBlockAckReqNeeded(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure) const = 0;
    virtual bool isBlockAckPolicyEligibleFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) const = 0;
    virtual std::tuple<MacAddress, SequenceNumberCyclic, Tid> computeBlockAckReqParameters(InProgressFrames *inProgressFrames, TxopProcedure *txopProcedure) const = 0;

    virtual simtime_t getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const = 0;
    virtual simtime_t getBlockAckTimeout(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& blockAckReq) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

