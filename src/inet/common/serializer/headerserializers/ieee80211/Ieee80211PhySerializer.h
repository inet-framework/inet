//
// Copyright (C) 2014 OpenSim Ltd.
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

#ifndef __INET_IEEE80211PHYSERIALIZER_H
#define __INET_IEEE80211PHYSERIALIZER_H

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211PLCPFrame_m.h"
#include "inet/common/BitVector.h"

namespace inet {
namespace serializer {

class INET_API Ieee80211PhySerializer
{
    protected:
        void writeToBitVector(unsigned char *buf, unsigned int bufSize, BitVector *bitVector) const;

    public:
        bool serialize(const inet::physicallayer::Ieee80211PLCPFrame *plcpHeader, BitVector *serializedPacket) const;
        inet::physicallayer::Ieee80211PLCPFrame *deserialize(BitVector *serializedPacket) const;
};

} /* namespace serializer */
} /* namespace inet */

#endif /* __INET_IEEE80211PHYSERIALIZER_H */
