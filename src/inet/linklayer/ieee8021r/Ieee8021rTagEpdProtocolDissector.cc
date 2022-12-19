//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021r/Ieee8021rTagEpdProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/ieee8021r/Ieee8021rTagHeader_m.h"

namespace inet {

namespace physicallayer {

Register_Protocol_Dissector(&Protocol::ieee8021rTag, Ieee8021rTagEpdProtocolDissector);

void Ieee8021rTagEpdProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<Ieee8021rTagEpdHeader>();
    callback.startProtocolDataUnit(protocol);
    callback.visitChunk(header, protocol);
    int typeOrLength = header->getTypeOrLength();
    if (isEth2Type(typeOrLength)) {
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength);
        callback.dissectPacket(packet, payloadProtocol);
    }
    else {
        auto ethEndOffset = packet->getFrontOffset() + B(typeOrLength);
        auto trailerOffset = packet->getBackOffset();
        packet->setBackOffset(ethEndOffset);
        callback.dissectPacket(packet, &Protocol::ieee8022llc);
        packet->setBackOffset(trailerOffset);
    }
    callback.endProtocolDataUnit(protocol);
}

} // namespace physicallayer

} // namespace inet

