// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IIEEE80211CHANNELACCESS_H
#define INET_IIEEE80211CHANNELACCESS_H
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211Channel.h"
namespace inet::physicallayer {
/** Physical channel observation; contention and TXOP decisions remain MAC-owned. */
class INET_API IIeee80211ChannelAccess
{
  public:
    virtual ~IIeee80211ChannelAccess() = default;
    virtual bool hasContiguousAllocation() const = 0;
    virtual const Ieee80211Channel *getOperatingChannel() const = 0;
    virtual bool isPrimaryChannelBusy() const = 0;
    virtual bool areSecondaryChannelsIdle(Hz width) const = 0;
};
}
#endif
