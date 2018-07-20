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
#include "inet/physicallayer/shortcut/ShortcutPhyProtocolDissector.h"
#include "inet/physicallayer/shortcut/ShortcutPhyHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::shortcutPhy, ShortcutPhyProtocolDissector);

void ShortcutPhyProtocolDissector::dissect(Packet *packet, ICallback& callback) const
{
    const auto& header = packet->popAtFront<ShortcutPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::shortcutPhy);
    callback.visitChunk(header, &Protocol::shortcutPhy);
    callback.dissectPacket(packet, header->getPayloadProtocol());
    callback.endProtocolDataUnit(&Protocol::shortcutPhy);
}

} // namespace inet

