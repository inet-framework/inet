//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/tcp_common/TcpProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::tcp, TcpProtocolDissector);

void TcpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<tcp::TcpHeader>();
    callback.startProtocolDataUnit(&Protocol::tcp);
    callback.visitChunk(header, &Protocol::tcp);
    if (packet->getDataLength() != b(0)) {
        auto dataProtocol = ProtocolGroup::getTcpProtocolGroup()->findProtocol(header->getDestPort());
        if (dataProtocol == nullptr)
            dataProtocol = ProtocolGroup::getTcpProtocolGroup()->findProtocol(header->getSrcPort());
        callback.dissectPacket(packet, dataProtocol);
    }
    callback.endProtocolDataUnit(&Protocol::tcp);
}

} // namespace inet

