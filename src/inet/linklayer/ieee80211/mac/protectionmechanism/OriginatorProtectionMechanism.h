//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ORIGINATORPROTECTIONMECHANISM_H
#define __INET_ORIGINATORPROTECTIONMECHANISM_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorProtectionMechanism : public ModeSetListener
{
  protected:
    IRateSelection *rateSelection = nullptr;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual simtime_t computeRtsDurationField(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader);
    virtual simtime_t computeDataFrameDurationField(Packet *dataPacket, const Ptr<const Ieee80211DataHeader>& dataHeader, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader);
    virtual simtime_t computeMgmtFrameDurationField(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader);

  public:
    virtual ~OriginatorProtectionMechanism() {}

    virtual simtime_t computeDurationField(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, Packet *pendingPacket, const Ptr<const Ieee80211DataOrMgmtHeader>& pendingHeader);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

