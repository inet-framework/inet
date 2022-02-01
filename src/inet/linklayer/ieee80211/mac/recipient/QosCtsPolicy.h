//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_QOSCTSPOLICY_H
#define __INET_QOSCTSPOLICY_H

#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"

namespace inet {
namespace ieee80211 {

class INET_API QosCtsPolicy : public ModeSetListener, public ICtsPolicy
{
  protected:
    IRx *rx = nullptr;
    IQosRateSelection *rateSelection = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual simtime_t computeCtsDuration(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const;

  public:
    virtual simtime_t computeCtsDurationField(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;
    virtual bool isCtsNeeded(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

