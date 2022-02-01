//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEDCACOLLISIONCONTROLLER_H
#define __INET_IEDCACOLLISIONCONTROLLER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class Edcaf;

class INET_API IEdcaCollisionController
{
  public:
    virtual ~IEdcaCollisionController() {}

    virtual void expectedChannelAccess(Edcaf *channelAccess, simtime_t time) = 0;
    virtual bool isInternalCollision(Edcaf *channelAccess) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

