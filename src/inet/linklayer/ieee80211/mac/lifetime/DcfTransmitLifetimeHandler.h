//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_DCFTRANSMITLIFETIMEHANDLER_H
#define __INET_DCFTRANSMITLIFETIMEHANDLER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/ITransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API DcfTransmitLifetimeHandler : public ITransmitLifetimeHandler
{
  protected:
    simtime_t maxTransmitLifetime;
    std::map<SequenceNumber, simtime_t> lifetimes;

  public:
    DcfTransmitLifetimeHandler(simtime_t maxTransmitLifetime) : maxTransmitLifetime(maxTransmitLifetime)
    {}

    virtual void frameGotInProgess(const Ptr<const Ieee80211DataHeader>& header);
    virtual void frameTransmitted(const Ptr<const Ieee80211DataHeader>& header);

    virtual bool isLifetimeExpired(const Ptr<const Ieee80211DataHeader>& header);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

