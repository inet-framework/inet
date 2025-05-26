//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_MCF_H
#define __INET_MCF_H

#include "inet/common/SimpleModule.h"
#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/framesequence/McfFs.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Mesh Coordination Function.
 */
class INET_API Mcf : public ICoordinationFunction, public SimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

  public:
    virtual void processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) override { throw cRuntimeError("Unimplemented!"); }
    virtual void processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override { throw cRuntimeError("Unimplemented!"); }
    virtual void corruptedFrameReceived() override { throw cRuntimeError("Unimplemented!"); }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

