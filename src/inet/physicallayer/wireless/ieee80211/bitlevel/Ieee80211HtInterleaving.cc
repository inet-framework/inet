//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211HtInterleaving.h"

namespace inet {
namespace physicallayer {

Ieee80211HtInterleaving::Ieee80211HtInterleaving(const std::vector<unsigned int>& numberOfCodedBitsPerSpatialStreams, Hz bandwidth) :
    numberOfCodedBitsPerSpatialStreams(numberOfCodedBitsPerSpatialStreams),
    bandwidth(bandwidth)
{
}

} /* namespace physicallayer */
} /* namespace inet */

