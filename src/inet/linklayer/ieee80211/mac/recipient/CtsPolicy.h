//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CTSPOLICY_H
#define __INET_CTSPOLICY_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"

namespace inet {
namespace ieee80211 {

class INET_API CtsPolicy : public ModeSetListener, public ICtsPolicy
{
  protected:
    IRateSelection *rateSelection = nullptr;
    IRx *rx = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual simtime_t computeCtsDuration(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const;

  public:
    virtual bool isCtsNeeded(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;
    virtual simtime_t computeCtsDurationField(Packet *packet, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

