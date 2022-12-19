//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/acking/AckingMacProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/acking/AckingMacHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ackingMac, AckingMacProtocolDissector);

void AckingMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<AckingMacHeader>();
    callback.startProtocolDataUnit(&Protocol::ackingMac);
    callback.visitChunk(header, &Protocol::ackingMac);
    auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(header->getNetworkProtocol());
    callback.dissectPacket(packet, payloadProtocol);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::ackingMac);
}

} // namespace inet

