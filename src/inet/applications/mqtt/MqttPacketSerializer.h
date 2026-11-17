// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTPACKETSERIALIZER_H
#define __INET_MQTTPACKETSERIALIZER_H

#include "inet/applications/mqtt/MqttPacket_m.h"
#include "inet/common/packet/serializer/FieldsChunkSerializer.h"

namespace inet::mqtt {

// The codec also serves the TCP framing boundary, where incomplete input is normal.
class INET_API MqttPacketSerializer : public FieldsChunkSerializer
{
  protected:
    void serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const override;
    const Ptr<Chunk> deserializeFields(MemoryInputStream& stream, const std::type_info&) const override;

  public:
    static bool isValidUtf8(const std::string& value);
    static std::vector<uint8_t> encode(const MqttPacket& packet);
    static Ptr<MqttPacket> decode(const std::vector<uint8_t>& bytes);
    // Zero means incomplete; malformed prefixes throw. No body allocation is needed.
    static size_t frameLength(const std::vector<uint8_t>& prefix);
    static void setLength(const Ptr<MqttPacket>& packet);
};

} // namespace inet::mqtt
#endif
