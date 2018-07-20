//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ITX_H
#define __INET_ITX_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

/**
 * Abstract interface for unconditionally transmitting a frame immediately
 * or after waiting for a specified inter-frame space (usually SIFS).
 */
class INET_API ITx
{
    public:
        class INET_API ICallback
        {
            public:
                virtual ~ICallback() { }

                virtual void transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) = 0;
        };

    public:
        virtual ~ITx() { }

        virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, ICallback *callback) = 0;
        virtual void transmitFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, simtime_t ifs, ICallback *callback) = 0;
        virtual void radioTransmissionFinished() = 0;
};

} // namespace ieee80211
} // namespace inet

#endif // #ifndef __INET_ITX_H
