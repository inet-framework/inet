// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inet/routing/aodv/AodvSubnetProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/routing/aodv/AodvControlPackets_m.h"
#include "inet/routing/aodv/AodvSubnetProtocol.h"

namespace inet {

Register_Protocol_Dissector(&aodv::aodvSubnetProtocol, AodvSubnetProtocolDissector);

void AodvSubnetProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& announcement = packet->popAtFront<aodv::AodvSubnetAnnouncement>();
    callback.startProtocolDataUnit(&aodv::aodvSubnetProtocol);
    callback.visitChunk(announcement, &aodv::aodvSubnetProtocol);
    callback.endProtocolDataUnit(&aodv::aodvSubnetProtocol);
}

} // namespace inet
