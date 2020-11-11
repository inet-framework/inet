//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

