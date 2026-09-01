//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IIEEE80211CHANNELPROVIDER_H
#define __INET_IIEEE80211CHANNELPROVIDER_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace physicallayer {

class Ieee80211Channel;

/**
 * Provides the current IEEE 802.11 channel of a packet-level PHY component.
 */
class INET_API IIeee80211ChannelProvider
{
  public:
    virtual ~IIeee80211ChannelProvider() {}

    virtual const Ieee80211Channel *getChannel() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif
