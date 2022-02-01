//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTQOSACKPOLICY_H
#define __INET_IRECIPIENTQOSACKPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class RecipientBlockAckAgreement;

class INET_API IRecipientQosAckPolicy
{
  public:
    virtual ~IRecipientQosAckPolicy() {}

    virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;
    virtual bool isBlockAckNeeded(const Ptr<const Ieee80211BlockAckReq>& blockAckReq, RecipientBlockAckAgreement *agreement) const = 0;

    virtual simtime_t computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;
    virtual simtime_t computeBasicBlockAckDurationField(Packet *packet, const Ptr<const Ieee80211BasicBlockAckReq>& basicBlockAckReq) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

