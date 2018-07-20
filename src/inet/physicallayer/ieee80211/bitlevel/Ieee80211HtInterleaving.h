//
// Copyright (C) 2015 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211HTINTERLEAVING_H
#define __INET_IEEE80211HTINTERLEAVING_H

#include "inet/physicallayer/contract/bitlevel/IInterleaver.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211HtInterleaving : public IInterleaving
{
    protected:
        // Let numberOfCodedBitsPerSpatialStreams.at(i) denote the number of coded bits for the ith spatial
        // stream and numberOfCodedBitsPerSpatialStreams.size() must be equal to N_SS (the number of spatial
        // streams).
        const std::vector<unsigned int>& numberOfCodedBitsPerSpatialStreams;
        const Hz bandwidth;

    public:
        virtual void printToStream(std::ostream& stream) const { stream << "Ieee80211HtInterleaving"; }
        Ieee80211HtInterleaving(const std::vector<unsigned int>& numberOfCodedBitsPerSpatialStreams, Hz bandwidth);
};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTINTERLEAVING_H
