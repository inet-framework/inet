// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSNPACKETSERIALIZER_H
#define __INET_MQTTSNPACKETSERIALIZER_H

#include "inet/applications/mqttsn/MqttSnPacket_m.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet::mqttsn {

class INET_API MqttSnPacketSerializer : public FieldsChunkSerializer
{
  protected:
    void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;

  public:
    static std::vector<uint8_t> encode(const MqttSnPacket& packet);
    static Ptr<MqttSnPacket> decode(const std::vector<uint8_t>& bytes);
    static void setLength(const Ptr<MqttSnPacket>& packet);
};

} // namespace inet::mqttsn
#endif
