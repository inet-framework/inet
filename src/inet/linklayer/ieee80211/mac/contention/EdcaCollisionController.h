//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EDCACOLLISIONCONTROLLER_H
#define __INET_EDCACOLLISIONCONTROLLER_H

#include "inet/linklayer/ieee80211/mac/contract/IEdcaCollisionController.h"

namespace inet {
namespace ieee80211 {

class INET_API EdcaCollisionController : public IEdcaCollisionController, public cSimpleModule
{
  protected:
    simtime_t txStartTimes[4];

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize() override;

  public:
    virtual void expectedChannelAccess(Edcaf *edcaf, simtime_t time) override;
    virtual bool isInternalCollision(Edcaf *edcaf) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

