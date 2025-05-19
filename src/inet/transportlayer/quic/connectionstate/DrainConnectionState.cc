//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "DrainConnectionState.h"
#include "../exception/ConnectionClosedException.h"

namespace inet {
namespace quic {

ConnectionState *DrainConnectionState::processOneRttPacket(const Ptr<const OneRttPacketHeader>& packetHeader, Packet *pkt)
{
    EV_DEBUG << "processOneRttPacket in " << name << endl;
    discardFrames(pkt);
    return this;
}

ConnectionState *DrainConnectionState::processConnectionCloseTimeout(cMessage *msg)
{
    throw ConnectionClosedException("DrainConnectionState: Waited long enough after CONNECTION_CLOSE frame. Destroy connection.");
}

} /* namespace quic */
} /* namespace inet */
