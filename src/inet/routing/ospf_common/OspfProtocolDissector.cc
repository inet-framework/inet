//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/routing/ospf_common/OspfProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/routing/ospf_common/OspfPacketBase_m.h"

namespace inet {
namespace ospf {

Register_Protocol_Dissector(&Protocol::ospf, OspfProtocolDissector);

void OspfProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto header = packet->popAtFront<OspfPacketBase>();
    callback.startProtocolDataUnit(&Protocol::ospf);
    callback.visitChunk(header, &Protocol::ospf);
    callback.endProtocolDataUnit(&Protocol::ospf);
}

} // namespace ospf
} // namespace inet

