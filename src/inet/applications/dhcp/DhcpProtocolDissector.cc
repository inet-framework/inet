//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/dhcp/DhcpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/applications/dhcp/DhcpMessage_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::dhcp, DhcpProtocolDissector);

void DhcpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& chunk = packet->popAtFront<DhcpMessage>();
    callback.startProtocolDataUnit(&Protocol::dhcp);
    callback.visitChunk(chunk, &Protocol::dhcp);
    callback.endProtocolDataUnit(&Protocol::dhcp);
}

} // namespace inet

