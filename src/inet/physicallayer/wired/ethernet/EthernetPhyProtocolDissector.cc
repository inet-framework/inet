//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wired/ethernet/EthernetPhyProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"
#include "inet/protocolelement/fragmentation/tag/FragmentTag_m.h"

namespace inet {

namespace physicallayer {

Register_Protocol_Dissector(&Protocol::ethernetPhy, EthernetPhyProtocolDissector);

void EthernetPhyProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<EthernetPhyHeaderBase>();
    callback.startProtocolDataUnit(&Protocol::ethernetPhy);
    callback.visitChunk(header, &Protocol::ethernetPhy);
    if (auto phyHeader = dynamicPtrCast<const EthernetFragmentPhyHeader>(header))
        // the Ethernet MAC protocol cannot be dissected here because this is just a fragment of a complete packet
        callback.dissectPacket(packet, nullptr);
    else
        callback.dissectPacket(packet, &Protocol::ethernetMac);
    callback.endProtocolDataUnit(&Protocol::ethernetPhy);
}

} // namespace physicallayer

} // namespace inet

