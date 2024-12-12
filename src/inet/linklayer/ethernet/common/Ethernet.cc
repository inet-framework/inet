//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ethernet/common/Ethernet.h"

#include "inet/common/checksum/EthernetCRC.h"

namespace inet {

uint32_t computeEthernetFcs(const Packet *packet, FcsMode fcsMode)
{
    switch (fcsMode) {
        case FCS_DISABLED:
            return 0x00000000L;
        case FCS_DECLARED_CORRECT:
            return 0xC00DC00DL;
        case FCS_DECLARED_INCORRECT:
            return 0xBAADBAADL;
        case FCS_COMPUTED: {
            auto data = packet->peekDataAsBytes();
            auto bytes = data->getBytes();
            return ethernetCRC(bytes.data(), bytes.size());
        }
        default:
            throw cRuntimeError("Unknown FCS mode: %d", (int)fcsMode);
    }
}

} // namespace inet
