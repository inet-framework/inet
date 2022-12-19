//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/common/ManetProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::manet, ManetProtocolDissector);

void ManetProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    callback.startProtocolDataUnit(&Protocol::manet);
//    callback.visitChunk(header, &Protocol::manet);
//    auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(header->getNetworkProtocol());
//    callback.dissectPacket(packet, payloadProtocol);
//    ASSERT(packet->getDataLength() == B(0));

    callback.dissectPacket(packet, nullptr); // KLUDGE choose from aodv|dsdv|dymo|gpsr

    callback.endProtocolDataUnit(&Protocol::manet);
}

} // namespace inet

