//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211VHTINTERLEAVING_H
#define __INET_IEEE80211VHTINTERLEAVING_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/IInterleaver.h"

namespace inet {
namespace physicallayer {

using namespace inet::units::values;

class INET_API Ieee80211VhtInterleaving : public IInterleaving
{
  protected:
    // Let numberOfCodedBitsPerSpatialStreams.at(i) denote the number of coded bits for the ith spatial
    // stream and numberOfCodedBitsPerSpatialStreams.size() must be equal to N_SS (the number of spatial
    // streams).
    const std::vector<unsigned int>& numberOfCodedBitsPerSpatialStreams;
    const Hz bandwidth;

  public:
    virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211HTInterleaving"; }
    Ieee80211VhtInterleaving(const std::vector<unsigned int>& numberOfCodedBitsPerSpatialStreams, Hz bandwidth);
};

} /* namespace physicallayer */
} /* namespace inet */

#endif

