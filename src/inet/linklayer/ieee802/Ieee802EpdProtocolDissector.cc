//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802/Ieee802EpdProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee802/Ieee802EpdHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee802epd, Ieee802EpdProtocolDissector);

void Ieee802EpdProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<Ieee802EpdHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee802epd);
    callback.visitChunk(header, &Protocol::ieee802epd);
    auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(header->getEtherType());
    callback.dissectPacket(packet, payloadProtocol);
    callback.endProtocolDataUnit(&Protocol::ieee802epd);
}

} // namespace inet

