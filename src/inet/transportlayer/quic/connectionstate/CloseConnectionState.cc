//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "CloseConnectionState.h"
#include "DrainConnectionState.h"
#include "../exception/ConnectionClosedException.h"

namespace inet {
namespace quic {

ConnectionState *CloseConnectionState::processHandshakePacket(const Ptr<const HandshakePacketHeader>& packetHeader, Packet *pkt) {
    EV_DEBUG << "processHandshakePacket in " << name << endl;
    discardFrames(pkt);
    return this;
}

ConnectionState *CloseConnectionState::processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt)
{
    EV_DEBUG << "processOneRttPacket in " << name << endl;

    if (containsFrame(pkt, FRAME_HEADER_TYPE_CONNECTION_CLOSE_QUIC)) {
        discardFrames(pkt);
        return new DrainConnectionState(context);
    }
    context->sendConnectionClose(false, false, 0);
    if (containsFrame(pkt, FRAME_HEADER_TYPE_CONNECTION_CLOSE_APP)) {
        discardFrames(pkt);
        return new DrainConnectionState(context);
    }
    discardFrames(pkt);
    return this;
}

ConnectionState *CloseConnectionState::processConnectionCloseTimeout(cMessage *msg)
{
    throw ConnectionClosedException("CloseConnectionState: Waited long enough after CONNECTION_CLOSE frame. Destroy connection.");
}

} /* namespace quic */
} /* namespace inet */
