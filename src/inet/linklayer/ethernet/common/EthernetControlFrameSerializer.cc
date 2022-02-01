//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/EthernetControlFrameSerializer.h"

#include <algorithm>

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ethernet/common/EthernetControlFrame_m.h"

namespace inet {

Register_Serializer(EthernetControlFrameBase, EthernetControlFrameSerializer);
Register_Serializer(EthernetPauseFrame, EthernetControlFrameSerializer);

void EthernetControlFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& frame = staticPtrCast<const EthernetControlFrameBase>(chunk);
    stream.writeUint16Be(frame->getOpCode());
    if (frame->getOpCode() == ETHERNET_CONTROL_PAUSE) {
        auto pauseFrame = dynamicPtrCast<const EthernetPauseFrame>(frame);
        ASSERT(pauseFrame != nullptr);
        stream.writeUint16Be(pauseFrame->getPauseTime());
    }
    else
        throw cRuntimeError("Cannot serialize '%s' (EthernetControlFrame with opCode = %d)", frame->getClassName(), frame->getOpCode());
}

const Ptr<Chunk> EthernetControlFrameSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<EthernetControlFrameBase> controlFrame = nullptr;
    uint16_t opCode = stream.readUint16Be();
    if (opCode == ETHERNET_CONTROL_PAUSE) {
        auto pauseFrame = makeShared<EthernetPauseFrame>();
        pauseFrame->setOpCode(opCode);
        pauseFrame->setPauseTime(stream.readUint16Be());
        controlFrame = pauseFrame;
    }
    else {
        controlFrame = makeShared<EthernetControlFrameBase>();
        controlFrame->setOpCode(opCode);
        controlFrame->markImproperlyRepresented();
    }
    return controlFrame;
}

} // namespace inet

