//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_DCFRECEIVELIFETIMEHANDLER_H
#define __INET_DCFRECEIVELIFETIMEHANDLER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API DcfReceiveLifetimeHandler
{
  protected:
    simtime_t maxReceiveLifetime;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

