//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/unitdisk/UnitDiskProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskPhyHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::unitDisk, UnitDiskProtocolDissector);

void UnitDiskProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<UnitDiskPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::unitDisk);
    callback.visitChunk(header, &Protocol::unitDisk);
    auto payloadProtocol = header->getPayloadProtocol();
    callback.dissectPacket(packet, payloadProtocol);
    ASSERT(packet->getDataLength() == B(0));
    callback.endProtocolDataUnit(&Protocol::unitDisk);
}

} // namespace inet

