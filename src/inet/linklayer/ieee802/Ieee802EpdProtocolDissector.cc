//
// Copyright (C) 2018 Raphael Riebl, TH Ingolstadt
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    auto payloadProtocol = ProtocolGroup::ethertype.findProtocol(header->getEtherType());
    callback.dissectPacket(packet, payloadProtocol);
    callback.endProtocolDataUnit(&Protocol::ieee802epd);
}

} // namespace inet

