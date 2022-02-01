//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTBLOCKACKAGREEMENTPOLICY_H
#define __INET_IRECIPIENTBLOCKACKAGREEMENTPOLICY_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientBlockAckAgreementPolicy
{
  public:
    virtual ~IRecipientBlockAckAgreementPolicy() {}

    virtual bool isAddbaReqAccepted(const Ptr<const Ieee80211AddbaRequest>& addbaReq) = 0;
    virtual bool isDelbaAccepted(const Ptr<const Ieee80211Delba>& delba) = 0;

    virtual simtime_t getBlockAckTimeoutValue() const = 0;
    virtual bool aMsduSupported() const = 0;
    virtual bool delayedBlockAckPolicySupported() const = 0;
    virtual int getMaximumAllowedBufferSize() const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

