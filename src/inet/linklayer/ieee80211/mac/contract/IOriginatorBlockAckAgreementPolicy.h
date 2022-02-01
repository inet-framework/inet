//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IORIGINATORBLOCKACKAGREEMENTPOLICY_H
#define __INET_IORIGINATORBLOCKACKAGREEMENTPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class OriginatorBlockAckAgreement;

class INET_API IOriginatorBlockAckAgreementPolicy
{
  public:
    virtual ~IOriginatorBlockAckAgreementPolicy() {}

    virtual bool isAddbaReqNeeded(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) = 0;
    virtual bool isAddbaReqAccepted(const Ptr<const Ieee80211AddbaResponse>& addbaResp, OriginatorBlockAckAgreement *agreement) = 0;
    virtual bool isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba) = 0;

    virtual bool isMsduSupported() const = 0;
    virtual simtime_t computeAddbaFailureTimeout() const = 0;
    virtual simtime_t getBlockAckTimeoutValue() const = 0;
    virtual bool isDelayedAckPolicySupported() const = 0;
    virtual int getMaximumAllowedBufferSize() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

