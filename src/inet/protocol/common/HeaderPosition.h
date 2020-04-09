//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_HEADERPOSITION_H
#define __INET_HEADERPOSITION_H

#include "inet/common/packet/Packet.h"

namespace inet {

enum HeaderPosition
{
    HP_UNDEFINED,
    HP_NONE,
    HP_FRONT,
    HP_BACK
};

INET_API HeaderPosition parseHeaderPosition(const char *string);

template <typename T>
const Ptr<const T> popHeader(Packet *packet, HeaderPosition headerPosition, b length)
{
    switch (headerPosition) {
        case HP_FRONT:
            return packet->popAtFront<T>();
        case HP_BACK:
            return packet->popAtBack<T>(length);
        default:
            throw cRuntimeError("Unknown headerPosition");
    }
}

template <typename T>
void insertHeader(Packet *packet, const Ptr<const T>& chunk, HeaderPosition headerPosition)
{
    switch (headerPosition) {
        case HP_FRONT:
            packet->insertAtFront(chunk);
            break;
        case HP_BACK:
            packet->insertAtBack(chunk);
            break;
        default:
            throw cRuntimeError("Unknown headerPosition");
    }
}

} // namespace inet

#endif // ifndef __INET_HEADERPOSITION_H

