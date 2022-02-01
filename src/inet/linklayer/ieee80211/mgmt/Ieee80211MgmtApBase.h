//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MGMTAPBASE_H
#define __INET_IEEE80211MGMTAPBASE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

namespace inet {

class EtherFrame;

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: abstract base class for management frame
 * handling for access points (APs). This class extends Ieee80211MgmtBase
 * with utility functions that are useful for implementing AP functionality.
 *
 */
class INET_API Ieee80211MgmtApBase : public Ieee80211MgmtBase
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;
};

} // namespace ieee80211

} // namespace inet

#endif

