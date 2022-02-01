//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORACKPOLICY_H
#define __INET_ORIGINATORACKPOLICY_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorAckPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorAckPolicy : public ModeSetListener, public IOriginatorAckPolicy
{
  protected:
    IRateSelection *rateSelection = nullptr;
    simtime_t ackTimeout = -1;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual bool isAckNeeded(const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
    virtual simtime_t getAckTimeout(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

