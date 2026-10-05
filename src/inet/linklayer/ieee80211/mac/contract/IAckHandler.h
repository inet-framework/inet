//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IACKHANDLER_H
#define __INET_IACKHANDLER_H

#include "inet/linklayer/ieee80211/mac/common/AckFrameState.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API IAckHandler
{
  public:
    virtual ~IAckHandler() {}

    // A missing registration is an error. This query must not insert an ACK entry.
    [[nodiscard]] virtual AckFrameState snapshotFrameState(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const = 0;

    virtual bool isEligibleToTransmit(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
    virtual bool isOutstandingFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
    virtual void frameGotInProgress(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) = 0;
    virtual void dropFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif
