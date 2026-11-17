// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttConnection.h"

#include "inet/common/packet/chunk/BytesChunk.h"

namespace inet::mqtt {

Ptr<MqttPacket> MqttConnection::extract(ChunkQueue& queue)
{
    if (queue.getLength() < B(2)) return nullptr;
    auto prefixLength = std::min(queue.getLength(), b(B(5)));
    auto prefix = queue.peek<BytesChunk>(prefixLength)->getBytes();
    auto length = MqttPacketSerializer::frameLength(prefix);
    if (!length || queue.getLength() < B(length)) return nullptr;
    auto bytes = queue.peek<BytesChunk>(B(length))->getBytes();
    auto packet = MqttPacketSerializer::decode(bytes);
    queue.pop(B(length));
    return packet;
}

} // namespace inet::mqtt
