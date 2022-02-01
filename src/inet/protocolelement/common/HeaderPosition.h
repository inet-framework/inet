//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_HEADERPOSITION_H
#define __INET_HEADERPOSITION_H

#include "inet/common/packet/Packet.h"

namespace inet {

enum HeaderPosition {
    HP_UNDEFINED,
    HP_NONE,
    HP_FRONT,
    HP_BACK
};

INET_API HeaderPosition parseHeaderPosition(const char *string);

template<typename T>
const Ptr<const T> peekHeader(const Packet *packet, HeaderPosition headerPosition, b length)
{
    switch (headerPosition) {
        case HP_FRONT:
            return packet->peekAtFront<T>();
        case HP_BACK:
            return packet->peekAtBack<T>(length);
        default:
            throw cRuntimeError("Unknown headerPosition");
    }
}

template<typename T>
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

template<typename T>
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

#endif

