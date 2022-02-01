//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECIPIENTACKPOLICY_H
#define __INET_IRECIPIENTACKPOLICY_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecipientAckPolicy
{
  public:
    virtual ~IRecipientAckPolicy() {}

    virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;
    virtual simtime_t computeAckDurationField(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

