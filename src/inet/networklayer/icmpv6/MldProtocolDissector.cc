//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/icmpv6/MldProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/networklayer/icmpv6/MldMessage_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::mld, MldProtocolDissector);

void MldProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    auto mldHeader = packet->popAtFront<MldMessage>();
    callback.startProtocolDataUnit(&Protocol::mld);
    callback.visitChunk(mldHeader, &Protocol::mld);
    callback.endProtocolDataUnit(&Protocol::mld);
}

} // namespace inet
