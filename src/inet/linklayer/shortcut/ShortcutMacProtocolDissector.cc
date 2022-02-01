//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/shortcut/ShortcutMacProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/shortcut/ShortcutMacHeader_m.h"

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

