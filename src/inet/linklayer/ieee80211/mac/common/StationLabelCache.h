//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STATIONLABELCACHE_H
#define __INET_STATIONLABELCACHE_H

#include <map>
#include <string>

#include "inet/linklayer/common/MacAddress.h"

namespace inet {
namespace ieee80211 {

/**
 * Resolves receiver MAC addresses to peer network node names, caching the result.
 *
 * The label identifies a station in a per-station signal emission: it names the details
 * object emitted with the value, which a demux() result filter and the statistic bar
 * chart visualizer key their per-station series on. Resolving sweeps the interface
 * tables of all network nodes, so it is done at most once per receiver.
 */
class INET_API StationLabelCache
{
  protected:
    std::map<MacAddress, std::string> labels;

  public:
    // The receiver's network node name if it can be resolved, otherwise the MAC address string.
    const std::string& getLabel(const MacAddress& receiver);
};

} // namespace ieee80211
} // namespace inet

#endif

