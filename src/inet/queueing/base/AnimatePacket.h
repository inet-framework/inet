//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ANIMATEPACKET_H
#define __INET_ANIMATEPACKET_H

#include "inet/common/packet/Packet.h"

namespace inet {
namespace queueing {

enum Action {
    PUSH,
    PULL,
};

INET_API void animate(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action);
INET_API void animatePacket(Packet *packet, cGate *startGate, cGate *endGate, Action action);
INET_API void animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId, Action action);
INET_API void animatePacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions, Action action);
INET_API void animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId, Action action);
INET_API void animatePacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions, Action action);
INET_API void animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId, Action action);
INET_API void animatePacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions, Action action);

INET_API void animatePush(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions);
INET_API void animatePushPacket(Packet *packet, cGate *startGate, cGate *endGate);
INET_API void animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId);
INET_API void animatePushPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions);
INET_API void animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId);
INET_API void animatePushPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions);
INET_API void animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId);
INET_API void animatePushPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions);

INET_API void animatePull(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions);
INET_API void animatePullPacket(Packet *packet, cGate *startGate, cGate *endGate);
INET_API void animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, long transmissionId);
INET_API void animatePullPacketStart(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, const SendOptions& sendOptions);
INET_API void animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, long transmissionId);
INET_API void animatePullPacketEnd(Packet *packet, cGate *startGate, cGate *endGate, const SendOptions& sendOptions);
INET_API void animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, long transmissionId);
INET_API void animatePullPacketProgress(Packet *packet, cGate *startGate, cGate *endGate, bps datarate, b position, b extraProcessableLength, const SendOptions& sendOptions);

} // namespace queueing
} // namespace inet

#endif

