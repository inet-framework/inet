//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211VhtInterleaving.h"

namespace inet {
namespace physicallayer {

Ieee80211VhtInterleaving::Ieee80211VhtInterleaving(const std::vector<unsigned int>& numberOfCodedBitsPerSpatialStreams, Hz bandwidth) :
    numberOfCodedBitsPerSpatialStreams(numberOfCodedBitsPerSpatialStreams),
    bandwidth(bandwidth)
{
}

} /* namespace physicallayer */
} /* namespace inet */

