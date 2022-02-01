//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8022/Ieee8022LlcProtocolDissector.h"

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"
#include "inet/linklayer/ieee8022/Ieee8022LlcHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ieee8022llc, Ieee802LlcDissector);

void Ieee802LlcDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<inet::Ieee8022LlcHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee8022llc);
    callback.visitChunk(header, &Protocol::ieee8022llc);
    auto dataProtocol = Ieee8022Llc::getProtocol(header);
    callback.dissectPacket(packet, dataProtocol);
    callback.endProtocolDataUnit(&Protocol::ieee8022llc);
}

} // namespace inet

