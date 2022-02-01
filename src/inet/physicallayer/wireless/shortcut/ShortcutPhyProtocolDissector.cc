//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

