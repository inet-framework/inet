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

