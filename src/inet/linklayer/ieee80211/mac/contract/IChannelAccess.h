//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ICHANNELACCESS_H
#define __INET_ICHANNELACCESS_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class INET_API IChannelAccess
{
  public:
    static simsignal_t channelOwnershipChangedSignal;

  public:
    class INET_API ICallback {
      public:
        virtual ~ICallback() {}

        virtual void channelGranted(IChannelAccess *channelAccess) = 0;
    };

  public:
    virtual ~IChannelAccess() {}

    virtual void requestChannel(ICallback *callback) = 0;
    virtual void releaseChannel(ICallback *callback) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

