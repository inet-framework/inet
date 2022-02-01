//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_HCCA_H
#define __INET_HCCA_H

#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Hybrid coordination function (HCF) Controlled Channel Access.
 */
class INET_API Hcca : public IChannelAccess, public cSimpleModule
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

  public:
    virtual bool isOwning();

    virtual void requestChannel(IChannelAccess::ICallback *callback) override;
    virtual void releaseChannel(IChannelAccess::ICallback *callback) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

