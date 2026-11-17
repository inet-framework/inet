// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqtt/MqttPacketSerializer.h"

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet::mqtt {

Register_Serializer(MqttPacket, MqttPacketSerializer);

namespace {
void require(bool condition, const char *reason)
{
    if (!condition)
        throw cRuntimeError("Malformed MQTT packet: %s", reason);
}

struct Writer {
    std::vector<uint8_t> data;
    void byte(unsigned value) { require(value <= 255, "byte overflow"); data.push_back(value); }
    void word(unsigned value) { require(value <= 65535, "word overflow"); byte(value >> 8); byte(value & 255); }
    void binary(const std::vector<uint8_t>& value) {
        require(value.size() <= 65535, "binary field exceeds two-byte length");
        word(value.size());
        data.insert(data.end(), value.begin(), value.end());
    }
    void text(const std::string& value) {
        require(MqttPacketSerializer::isValidUtf8(value), "invalid UTF-8");
        binary(std::vector<uint8_t>(value.begin(), value.end()));
    }
};

struct Reader {
    const std::vector<uint8_t>& data;
    size_t position = 0;
    unsigned byte() { require(position < data.size(), "truncated field"); return data[position++]; }
    unsigned word() { unsigned high = byte(); return (high << 8) | byte(); }
    std::vector<uint8_t> binary() {
        auto length = word();
        require(length <= data.size() - position, "truncated binary field");
        std::vector<uint8_t> value(data.begin() + position, data.begin() + position + length);
        position += length;
        return value;
    }
    std::string text() {
        auto value = binary();
        std::string text(value.begin(), value.end());
        require(MqttPacketSerializer::isValidUtf8(text), "invalid UTF-8");
        return text;
    }
};
}

bool MqttPacketSerializer::isValidUtf8(const std::string& value)
{
    // MQTT 3.1.1 section 1.5.3: reject NUL, surrogates and invalid/overlong UTF-8.
    size_t i = 0;
    while (i < value.size()) {
        unsigned lead = static_cast<uint8_t>(value[i++]);
        unsigned code = lead;
        unsigned count = 0;
        unsigned minimum = 0;
        if (lead >= 0xc2 && lead <= 0xdf) { code = lead & 31; count = 1; minimum = 0x80; }
        else if (lead >= 0xe0 && lead <= 0xef) { code = lead & 15; count = 2; minimum = 0x800; }
        else if (lead >= 0xf0 && lead <= 0xf4) { code = lead & 7; count = 3; minimum = 0x10000; }
        else if (lead >= 0x80) return false;
        if (count > value.size() - i) return false;
        for (unsigned j = 0; j < count; j++) {
            unsigned next = static_cast<uint8_t>(value[i++]);
            if ((next & 0xc0) != 0x80) return false;
            code = (code << 6) | (next & 63);
        }
        if (code < minimum || code == 0 || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff)) return false;
    }
    return true;
}

size_t MqttPacketSerializer::frameLength(const std::vector<uint8_t>& prefix)
{
    // Section 2.2.3: Remaining Length has at most four base-128 digits.
    size_t remaining = 0, multiplier = 1;
    for (size_t i = 1; i <= 4; i++) {
        if (prefix.size() <= i) return 0;
        remaining += (prefix[i] & 127) * multiplier;
        if (!(prefix[i] & 128)) return 1 + i + remaining;
        require(i < 4, "Remaining Length exceeds four bytes");
        multiplier *= 128;
    }
    return 0;
}

std::vector<uint8_t> MqttPacketSerializer::encode(const MqttPacket& packet)
{
    Writer body;
    auto identifier = [&]() {
        require(packet.getPacketIdentifier() > 0, "zero packet identifier");
        body.word(packet.getPacketIdentifier());
    };
    unsigned flags = 0;
    switch (packet.getType()) {
        case CONNECT: {
            body.text(packet.getProtocolName());
            body.byte(packet.getProtocolLevel());
            require(packet.getWillQos() >= 0 && packet.getWillQos() <= 2, "Will QoS");
            require(packet.getWill() || (!packet.getWillRetain() && packet.getWillQos() == 0), "Will flags");
            require(!packet.getPasswordPresent() || packet.getUserNamePresent(), "password without username");
            body.byte((packet.getUserNamePresent() ? 128 : 0) | (packet.getPasswordPresent() ? 64 : 0) |
                    (packet.getWillRetain() ? 32 : 0) | (packet.getWillQos() << 3) |
                    (packet.getWill() ? 4 : 0) | (packet.getCleanSession() ? 2 : 0));
            body.word(packet.getKeepAlive());
            body.text(packet.getClientIdentifier());
            if (packet.getWill()) {
                body.text(packet.getWillTopic());
                std::vector<uint8_t> value;
                for (size_t i = 0; i < packet.getWillPayloadArraySize(); i++) value.push_back(packet.getWillPayload(i));
                body.binary(value);
            }
            if (packet.getUserNamePresent()) body.text(packet.getUserName());
            if (packet.getPasswordPresent()) {
                std::vector<uint8_t> value;
                for (size_t i = 0; i < packet.getPasswordArraySize(); i++) value.push_back(packet.getPassword(i));
                body.binary(value);
            }
            break;
        }
        case CONNACK:
            require(packet.getReturnCode() >= 0 && packet.getReturnCode() <= 5, "CONNACK return code");
            require(packet.getReturnCode() == 0 || !packet.getSessionPresent(), "refused session present");
            body.byte(packet.getSessionPresent()); body.byte(packet.getReturnCode()); break;
        case PUBLISH:
            require(packet.getQos() >= 0 && packet.getQos() <= 2, "PUBLISH QoS");
            require(packet.getQos() != 0 || !packet.getDuplicate(), "QoS 0 DUP");
            flags = (packet.getDuplicate() ? 8 : 0) | (packet.getQos() << 1) | packet.getRetain();
            body.text(packet.getTopic());
            if (packet.getQos()) identifier();
            for (size_t i = 0; i < packet.getPayloadArraySize(); i++) body.byte(packet.getPayload(i));
            break;
        case PUBREL: flags = 2; identifier(); break;
        case PUBACK: case PUBREC: case PUBCOMP: case UNSUBACK: identifier(); break;
        case SUBSCRIBE: case UNSUBSCRIBE:
            flags = 2; identifier();
            require(packet.getFiltersArraySize() > 0, "empty subscription list");
            require(packet.getType() != SUBSCRIBE || packet.getResultsArraySize() == packet.getFiltersArraySize(), "subscription QoS count");
            for (size_t i = 0; i < packet.getFiltersArraySize(); i++) {
                body.text(packet.getFilters(i));
                if (packet.getType() == SUBSCRIBE) {
                    require(packet.getResults(i) >= 0 && packet.getResults(i) <= 2, "requested QoS");
                    body.byte(packet.getResults(i));
                }
            }
            break;
        case SUBACK:
            identifier();
            require(packet.getResultsArraySize() > 0, "empty SUBACK");
            for (size_t i = 0; i < packet.getResultsArraySize(); i++) {
                auto result = packet.getResults(i);
                require(result == 0 || result == 1 || result == 2 || result == 128, "SUBACK result");
                body.byte(result);
            }
            break;
        case PINGREQ: case PINGRESP: case DISCONNECT: break;
        default: throw cRuntimeError("Unknown MQTT packet type");
    }
    require(body.data.size() <= 268435455, "Remaining Length overflow");
    Writer wire;
    wire.byte((packet.getType() << 4) | flags);
    size_t length = body.data.size();
    do {
        unsigned digit = length % 128;
        length /= 128;
        wire.byte(digit | (length ? 128 : 0));
    } while (length);
    wire.data.insert(wire.data.end(), body.data.begin(), body.data.end());
    return wire.data;
}

Ptr<MqttPacket> MqttPacketSerializer::decode(const std::vector<uint8_t>& bytes)
{
    require(bytes.size() >= 2 && frameLength(bytes) == bytes.size(), "incomplete packet or trailing bytes");
    Reader input{bytes};
    unsigned header = input.byte();
    while (input.byte() & 128) {}
    auto packet = makeShared<MqttPacket>();
    packet->setType(header >> 4);
    unsigned flags = header & 15;
    unsigned expected = packet->getType() == PUBREL || packet->getType() == SUBSCRIBE || packet->getType() == UNSUBSCRIBE ? 2 : 0;
    require(packet->getType() == PUBLISH || flags == expected, "fixed header flags");
    auto identifier = [&]() { auto id = input.word(); require(id > 0, "zero packet identifier"); packet->setPacketIdentifier(id); };
    switch (packet->getType()) {
        case CONNECT: {
            packet->setProtocolName(input.text().c_str()); packet->setProtocolLevel(input.byte());
            auto connectFlags = input.byte();
            require(!(connectFlags & 1), "CONNECT reserved bit");
            packet->setCleanSession(connectFlags & 2); packet->setWill(connectFlags & 4);
            packet->setWillQos((connectFlags >> 3) & 3); packet->setWillRetain(connectFlags & 32);
            packet->setUserNamePresent(connectFlags & 128); packet->setPasswordPresent(connectFlags & 64);
            packet->setKeepAlive(input.word()); packet->setClientIdentifier(input.text().c_str());
            if (packet->getWill()) {
                packet->setWillTopic(input.text().c_str()); auto value = input.binary();
                packet->setWillPayloadArraySize(value.size());
                for (size_t i = 0; i < value.size(); i++) packet->setWillPayload(i, value[i]);
            }
            if (packet->getUserNamePresent()) packet->setUserName(input.text().c_str());
            if (packet->getPasswordPresent()) {
                auto value = input.binary(); packet->setPasswordArraySize(value.size());
                for (size_t i = 0; i < value.size(); i++) packet->setPassword(i, value[i]);
            }
            break;
        }
        case CONNACK: {
            auto acknowledgeFlags = input.byte(); require(acknowledgeFlags <= 1, "CONNACK flags");
            packet->setSessionPresent(acknowledgeFlags); packet->setReturnCode(input.byte()); break;
        }
        case PUBLISH: {
            packet->setDuplicate(flags & 8); packet->setQos((flags >> 1) & 3); packet->setRetain(flags & 1);
            packet->setTopic(input.text().c_str());
            if (packet->getQos()) identifier();
            auto length = bytes.size() - input.position; packet->setPayloadArraySize(length);
            for (size_t i = 0; i < length; i++) packet->setPayload(i, input.byte());
            break;
        }
        case PUBACK: case PUBREC: case PUBREL: case PUBCOMP: case UNSUBACK: identifier(); break;
        case SUBSCRIBE: case UNSUBSCRIBE:
            identifier();
            while (input.position < bytes.size()) {
                auto index = packet->getFiltersArraySize(); auto filter = input.text();
                packet->setFiltersArraySize(index + 1); packet->setFilters(index, filter.c_str());
                if (packet->getType() == SUBSCRIBE) {
                    packet->setResultsArraySize(index + 1); packet->setResults(index, input.byte());
                }
            }
            break;
        case SUBACK:
            identifier();
            while (input.position < bytes.size()) {
                auto index = packet->getResultsArraySize(); packet->setResultsArraySize(index + 1);
                packet->setResults(index, input.byte());
            }
            break;
        case PINGREQ: case PINGRESP: case DISCONNECT: break;
        default: throw cRuntimeError("Malformed MQTT packet: reserved type");
    }
    require(input.position == bytes.size(), "unexpected packet body");
    // Reuse field validation for both directions, without requiring identical length encoding.
    encode(*packet);
    packet->setChunkLength(B(bytes.size()));
    return packet;
}

void MqttPacketSerializer::setLength(const Ptr<MqttPacket>& packet)
{
    packet->setChunkLength(B(encode(*packet).size()));
}

void MqttPacketSerializer::serializeFields(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    for (auto byte : encode(*staticPtrCast<const MqttPacket>(chunk))) stream.writeByte(byte);
}

const Ptr<Chunk> MqttPacketSerializer::deserializeFields(MemoryInputStream& stream, const std::type_info&) const
{
    std::vector<uint8_t> bytes{stream.readByte()};
    do { bytes.push_back(stream.readByte()); } while (!frameLength(bytes));
    size_t length = frameLength(bytes);
    require(B(length - bytes.size()) <= stream.getRemainingLength(), "truncated packet");
    while (bytes.size() < length) bytes.push_back(stream.readByte());
    return decode(bytes);
}

} // namespace inet::mqtt
