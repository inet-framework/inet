//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/generic/GenericProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wireless/generic/GenericPhyHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::genericPhy, GenericProtocolDissector);

void GenericProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<GenericPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::genericPhy);
    callback.visitChunk(header, &Protocol::genericPhy);
    auto payloadProtocol = header->getPayloadProtocol();
    callback.dissectPacket(packet, payloadProtocol);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::genericPhy);
}

} // namespace inet

