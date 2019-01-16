//
// Copyright (C) OpenSim Ltd
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
#include "inet/linklayer/shortcut/ShortcutMacHeader_m.h"
#include "inet/linklayer/shortcut/ShortcutMacProtocolDissector.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::shortcutMac, ShortcutMacProtocolDissector);

void ShortcutMacProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<ShortcutMacHeader>();
    callback.startProtocolDataUnit(&Protocol::shortcutMac);
    callback.visitChunk(header, &Protocol::shortcutMac);
    callback.dissectPacket(packet, header->getPayloadProtocol());
    callback.endProtocolDataUnit(&Protocol::shortcutMac);
}

} // namespace inet

