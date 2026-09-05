//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IIEEE80211HTCHANNELWIDTHPROVIDER_H
#define __INET_IIEEE80211HTCHANNELWIDTHPROVIDER_H

#include "inet/common/Units.h"

namespace inet {
namespace physicallayer {

using namespace inet::units::values;

/**
 * Reports HT channel widths that an IEEE 802.11 PHY component can actually
 * operate, independently of the modes described by its mode catalog.
 */
class INET_API IIeee80211HtChannelWidthProvider
{
  public:
    virtual ~IIeee80211HtChannelWidthProvider() {}

    virtual bool isHtChannelWidthSupported(Hz channelWidth) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif
