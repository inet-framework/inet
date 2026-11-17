// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnPacketSerializer.h"
#include "inet/applications/mqttsn/MqttSnProtocol.h"
#include "inet/applications/mqtt/MqttPacketSerializer.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet::mqttsn {

Register_Serializer(MqttSnPacket, MqttSnPacketSerializer);

namespace {
void require(bool condition, const char *reason)
{
    if (!condition) throw cRuntimeError("Invalid MQTT-SN message: %s", reason);
}

struct Reader
{
    const std::vector<uint8_t>& bytes;
    size_t position;
    size_t left() const { return bytes.size() - position; }
    int byte() { require(left() >= 1, "truncated field"); return bytes[position++]; }
    int word() { int high = byte(); return high * 256 + byte(); }
    std::string text() {
        std::string value(bytes.begin() + position, bytes.end());
        position = bytes.size();
        require(mqtt::MqttPacketSerializer::isValidUtf8(value), "UTF-8");
        return value;
    }
};

void checkText(const std::string& text)
{
    require(mqtt::MqttPacketSerializer::isValidUtf8(text), "UTF-8");
}

void checkClient(const std::string& text, bool optional)
{
    checkText(text);
    size_t count = 0;
    for (unsigned char value : text) if ((value & 0xc0) != 0x80) count++;
    require((optional && count == 0) || (count >= 1 && count <= 23), "ClientId length");
}
}

std::vector<uint8_t> MqttSnPacketSerializer::encode(const MqttSnPacket& packet)
{
    // MQTT-SN v1.2 sections 5.2–5.4: lengths and integer fields are network order.
    std::vector<uint8_t> body;
    auto byte = [&](int value) { require(value >= 0 && value <= 255, "octet range"); body.push_back(value); };
    auto word = [&](int value) { require(value >= 0 && value <= 65535, "word range"); byte(value >> 8); byte(value & 255); };
    auto text = [&](const std::string& value) { checkText(value); body.insert(body.end(), value.begin(), value.end()); };
    auto payload = [&]() { for (size_t i = 0; i < packet.getPayloadArraySize(); i++) byte(packet.getPayload(i)); };
    auto code = [&]() { require(packet.getReturnCode() >= 0 && packet.getReturnCode() <= 3, "return code"); byte(packet.getReturnCode()); };
    auto flags = [&]() { byte(packet.getFlags()); };
    auto topic = [&]() {
        require(topicKind(packet) != 3, "reserved topic representation");
        if (topicKind(packet) == NORMAL_TOPIC) text(packet.getTopic());
        else word(packet.getTopicId());
    };
    switch (packet.getType()) {
        case ADVERTISE: byte(packet.getGatewayId()); word(packet.getDuration()); break;
        case SEARCHGW: byte(packet.getRadius()); break;
        case GWINFO:
            byte(packet.getGatewayId());
            for (size_t i = 0; i < packet.getGatewayAddressArraySize(); i++) byte(packet.getGatewayAddress(i));
            break;
        case CONNECT:
            flags(); byte(packet.getProtocolId()); word(packet.getDuration());
            checkClient(packet.getClientIdentifier(), false); text(packet.getClientIdentifier()); break;
        case CONNACK: case WILLTOPICRESP: case WILLMSGRESP: code(); break;
        case WILLTOPIC: case WILLTOPICUPD:
            if (packet.getWillTopicPresent()) { flags(); text(packet.getTopic()); require(*packet.getTopic(), "empty Will topic"); }
            break;
        case WILLMSG: case WILLMSGUPD: payload(); break;
        case REGISTER: require(packet.getMessageId() > 0, "REGISTER message id"); word(packet.getTopicId()); word(packet.getMessageId()); text(packet.getTopic()); break;
        case REGACK: require(packet.getMessageId() > 0, "REGACK message id"); word(packet.getTopicId()); word(packet.getMessageId()); code(); break;
        case PUBACK: word(packet.getTopicId()); word(packet.getMessageId()); code(); break;
        case PUBLISH:
            require(topicKind(packet) != 3, "reserved topic representation");
            require(qos(packet) > 0 || packet.getMessageId() == 0, "QoS 0/-1 message id");
            require(qos(packet) <= 0 || packet.getMessageId() > 0, "QoS 1/2 message id");
            flags(); word(packet.getTopicId()); word(packet.getMessageId()); payload(); break;
        case PUBREC: case PUBREL: case PUBCOMP: case UNSUBACK:
            require(packet.getMessageId() > 0, "response message id"); word(packet.getMessageId()); break;
        case SUBSCRIBE: case UNSUBSCRIBE:
            require(packet.getMessageId() > 0, "subscription message id"); flags(); word(packet.getMessageId()); topic(); break;
        case SUBACK: require(packet.getMessageId() > 0, "SUBACK message id"); flags(); word(packet.getTopicId()); word(packet.getMessageId()); code(); break;
        case PINGREQ: checkClient(packet.getClientIdentifier(), true); text(packet.getClientIdentifier()); break;
        case DISCONNECT: if (packet.getDurationPresent()) word(packet.getDuration()); break;
        case PINGRESP: case WILLTOPICREQ: case WILLMSGREQ: break;
        default: throw cRuntimeError("Unsupported MQTT-SN message type %d", packet.getType());
    }
    bool extended = packet.getExtendedLength() || body.size() + 2 > 255;
    size_t length = body.size() + (extended ? 4 : 2);
    require(length <= 65535, "message length");
    std::vector<uint8_t> result;
    if (extended) { result.push_back(1); result.push_back(length >> 8); result.push_back(length & 255); }
    else result.push_back(length);
    result.push_back(packet.getType());
    result.insert(result.end(), body.begin(), body.end());
    return result;
}

Ptr<MqttSnPacket> MqttSnPacketSerializer::decode(const std::vector<uint8_t>& bytes)
{
    require(bytes.size() >= 2, "short header");
    Reader input{bytes, 0};
    int length = input.byte();
    bool extended = length == 1;
    if (extended) length = input.word();
    require(length == (int)bytes.size(), "datagram length mismatch");
    auto packet = makeShared<MqttSnPacket>();
    packet->setExtendedLength(extended); packet->setType(input.byte());
    auto flags = [&]() { packet->setFlags(input.byte()); };
    auto ids = [&]() { packet->setTopicId(input.word()); packet->setMessageId(input.word()); };
    auto code = [&]() { packet->setReturnCode(input.byte()); };
    auto payload = [&]() {
        packet->setPayloadArraySize(input.left());
        for (size_t i = 0; i < packet->getPayloadArraySize(); i++) packet->setPayload(i, input.byte());
    };
    switch (packet->getType()) {
        case ADVERTISE: packet->setGatewayId(input.byte()); packet->setDuration(input.word()); break;
        case SEARCHGW: packet->setRadius(input.byte()); break;
        case GWINFO:
            packet->setGatewayId(input.byte()); packet->setGatewayAddressArraySize(input.left());
            for (size_t i = 0; i < packet->getGatewayAddressArraySize(); i++) packet->setGatewayAddress(i, input.byte());
            break;
        case CONNECT:
            flags(); packet->setProtocolId(input.byte()); packet->setDuration(input.word());
            packet->setClientIdentifier(input.text().c_str()); break;
        case CONNACK: case WILLTOPICRESP: case WILLMSGRESP: code(); break;
        case WILLTOPIC: case WILLTOPICUPD:
            packet->setWillTopicPresent(input.left() > 0);
            if (packet->getWillTopicPresent()) { flags(); packet->setTopic(input.text().c_str()); }
            break;
        case WILLMSG: case WILLMSGUPD: payload(); break;
        case REGISTER: ids(); packet->setTopic(input.text().c_str()); break;
        case REGACK: case PUBACK: ids(); code(); break;
        case PUBLISH: flags(); ids(); payload(); break;
        case PUBREC: case PUBREL: case PUBCOMP: case UNSUBACK: packet->setMessageId(input.word()); break;
        case SUBSCRIBE: case UNSUBSCRIBE:
            flags(); packet->setMessageId(input.word());
            if (topicKind(*packet) == NORMAL_TOPIC) packet->setTopic(input.text().c_str());
            else packet->setTopicId(input.word());
            break;
        case SUBACK: flags(); ids(); code(); break;
        case PINGREQ: packet->setClientIdentifier(input.text().c_str()); break;
        case DISCONNECT:
            packet->setDurationPresent(input.left() > 0);
            if (packet->getDurationPresent()) packet->setDuration(input.word());
            break;
        case PINGRESP: case WILLTOPICREQ: case WILLMSGREQ: break;
        default: throw cRuntimeError("Unsupported MQTT-SN message type %d", packet->getType());
    }
    require(input.left() == 0, "trailing fields");
    // Reuse outgoing field validation, without normalizing raw flags or length form.
    require(encode(*packet) == bytes, "invalid field encoding");
    packet->setChunkLength(B(bytes.size()));
    return packet;
}

void MqttSnPacketSerializer::setLength(const Ptr<MqttSnPacket>& packet)
{
    auto bytes = encode(*packet);
    packet->setExtendedLength(bytes[0] == 1);
    packet->setChunkLength(B(bytes.size()));
}

void MqttSnPacketSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto bytes = encode(*staticPtrCast<const MqttSnPacket>(chunk));
    stream.writeBytes(bytes);
}

const Ptr<Chunk> MqttSnPacketSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    std::vector<uint8_t> bytes;
    int first = stream.readByte(); bytes.push_back(first);
    size_t length = first;
    if (first == 1) {
        int high = stream.readByte(), low = stream.readByte();
        bytes.push_back(high); bytes.push_back(low); length = high * 256 + low;
    }
    require(length >= bytes.size() + 1 && length <= 65535, "header length");
    require(B(length - bytes.size()) <= stream.getRemainingLength(), "truncated datagram");
    while (bytes.size() < length) bytes.push_back(stream.readByte());
    return decode(bytes);
}

} // namespace inet::mqttsn
