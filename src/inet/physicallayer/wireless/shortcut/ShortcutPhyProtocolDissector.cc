//
// Copyright (C) 2020 OpenSim Ltd.
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

#include "inet/physicallayer/wireless/shortcut/ShortcutPhyProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/physicallayer/wireless/shortcut/ShortcutPhyHeader_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::shortcutPhy, ShortcutPhyProtocolDissector);

void ShortcutPhyProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<ShortcutPhyHeader>();
    callback.startProtocolDataUnit(&Protocol::shortcutPhy);
    callback.visitChunk(header, &Protocol::shortcutPhy);
    callback.dissectPacket(packet, header->getPayloadProtocol());
    callback.endProtocolDataUnit(&Protocol::shortcutPhy);
}

} // namespace inet

