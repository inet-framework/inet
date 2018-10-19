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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeHeader_m.h"
#include "inet/linklayer/ieee80211/llc/Ieee80211EtherTypeProtocolDissector.h"

namespace inet {
namespace ieee80211 {

Register_Protocol_Dissector(&Protocol::ieee80211EtherType, Ieee80211EtherTypeProtocolDissector);

void Ieee80211EtherTypeProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<inet::Ieee80211EtherTypeHeader>();
    callback.startProtocolDataUnit(&Protocol::ieee80211EtherType);
    callback.visitChunk(header, &Protocol::ieee80211EtherType);
    callback.dissectPacket(packet, header->getProtocol());
    callback.endProtocolDataUnit(&Protocol::ieee80211EtherType);
}

} // namespace ieee80211
} // namespace inet

