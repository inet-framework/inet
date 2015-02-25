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

#ifndef __INET_IEEE80211SERIALIZER_H
#define __INET_IEEE80211SERIALIZER_H

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/common/serializer/headers/defs.h"

/**
 * Converts between Ieee802.11Frame and binary (network byte order)  Ieee802.11 header.
 */
namespace inet {
namespace serializer {

using namespace ieee80211;

class INET_API Ieee80211Serializer
{
    public:
        /**
         * Serializes an Ieee802.11Frame for transmission on the network card.
         * Returns the length of data written into buffer.
         */
        int serialize(Ieee80211Frame *pkt, unsigned char *buf, unsigned int bufsize);

        /**
         * Puts a packet sniffed from the network card into an Ieee802.11Frame.
         */
        cPacket* parse(const unsigned char *buf, unsigned int bufsize);
};

} // namespace serializer

} // namespace inet

#endif /* __INET_IEEE80211SERIALIZER_H */
