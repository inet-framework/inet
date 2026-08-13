//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/EthernetFlowControlProtocolDissector.h"

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"

namespace inet {

Register_Protocol_Dissector(&Protocol::ethernetFlowCtrl, EthernetFlowControlProtocolDissector);

void EthernetFlowControlProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    // The deserializer dispatches on the opcode and returns the concrete frame (a PAUSE
    // frame for opcode 1, the base class marked improperly represented otherwise), so
    // the length it consumes varies; what is left is the padding to the minimum frame
    // size, which the Ethernet dissector visits.
    const auto& controlFrame = packet->popAtFront<EthernetControlFrameBase>();
    callback.startProtocolDataUnit(&Protocol::ethernetFlowCtrl);
    callback.visitChunk(controlFrame, &Protocol::ethernetFlowCtrl);
    callback.endProtocolDataUnit(&Protocol::ethernetFlowCtrl);
}

} // namespace inet
