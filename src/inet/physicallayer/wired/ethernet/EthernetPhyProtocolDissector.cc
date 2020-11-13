//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

