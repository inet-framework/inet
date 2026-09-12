//
// Copyright (C) 2026 INET Framework contributors
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211RADIO_H
#define __INET_IIEEE80211RADIO_H

#include "inet/common/INETDefs.h"
#include "inet/common/Units.h"

namespace inet {
namespace physicallayer {

class Ieee80211Channel;

/** Read-only IEEE 802.11 PHY capabilities, in addition to the IRadio role. */
class INET_API IIeee80211Radio
{
  public:
    virtual ~IIeee80211Radio() = default;
    // True only when both the operational transmitter and receiver support this width.
    virtual bool isHtChannelWidthSupported(units::values::Hz channelWidth) const = 0;
    // Borrowed channel, or nullptr when no channel is configured.
    virtual const Ieee80211Channel *getChannel() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif
