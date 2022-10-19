//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/ipv4/IgmpProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/ipv4/IgmpMessage_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::igmp, IgmpProtocolDissector);

void IgmpProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto igmpPacket = packet->popAtFront<IgmpMessage>();
    callback.startProtocolDataUnit(&Protocol::igmp);
    callback.visitChunk(igmpPacket, &Protocol::igmp);
    callback.endProtocolDataUnit(&Protocol::igmp);
}

} // namespace inet

