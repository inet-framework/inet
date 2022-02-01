//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EDCATRANSMITLIFETIMEHANDLER_H
#define __INET_EDCATRANSMITLIFETIMEHANDLER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/contract/ITransmitLifetimeHandler.h"

namespace inet {
namespace ieee80211 {

class INET_API EdcaTransmitLifetimeHandler : public ITransmitLifetimeHandler
{
  public:
    simtime_t msduLifetime[4];
    std::map<SequenceNumber, simtime_t> lifetimes;

  protected:
    AccessCategory mapTidToAc(int tid); // TODO copy

  public:
    EdcaTransmitLifetimeHandler(simtime_t bkLifetime, simtime_t beLifetime, simtime_t viLifetime, simtime_t voLifetime);

    virtual void frameGotInProgess(const Ptr<const Ieee80211DataHeader>& header);
    virtual void frameTransmitted(const Ptr<const Ieee80211DataHeader>& header);

    virtual bool isLifetimeExpired(const Ptr<const Ieee80211DataHeader>& header);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

