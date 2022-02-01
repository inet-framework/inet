//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021d/stp/StpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee8021d/common/Ieee8021dBpdu_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::stp, StpProtocolDissector);

void StpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto stpPacket = packet->popAtFront<BpduBase>();
    callback.startProtocolDataUnit(&Protocol::stp);
    callback.visitChunk(stpPacket, &Protocol::stp);
    callback.endProtocolDataUnit(&Protocol::stp);
}

} // namespace inet

