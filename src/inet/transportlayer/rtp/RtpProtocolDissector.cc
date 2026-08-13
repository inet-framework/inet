//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/transportlayer/rtp/RtpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/transportlayer/rtp/RtpPacket_m.h"

namespace inet {
namespace rtp {

Register_Protocol_Dissector(&Protocol::rtp, RtpProtocolDissector);

void RtpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<RtpHeader>();
    callback.startProtocolDataUnit(&Protocol::rtp);
    callback.visitChunk(header, &Protocol::rtp);
    // the media payload is opaque: what it holds is named by the payload type, and the
    // codec formats are not modelled
    if (packet->getDataLength() > b(0))
        callback.dissectPacket(packet, nullptr);
    callback.endProtocolDataUnit(&Protocol::rtp);
}

} // namespace rtp
} // namespace inet
