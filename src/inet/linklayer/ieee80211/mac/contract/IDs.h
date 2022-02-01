//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IDS_H
#define __INET_IDS_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for distribution service.
 */
class INET_API IDs
{
  public:
    virtual ~IDs() {}

    virtual void processDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& header) = 0;
};

} // namespace ieee80211
} // namespace inet

#endif

