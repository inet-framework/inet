// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/applications/mqttsn/MqttSnSession.h"
#include "inet/applications/mqttsn/MqttSnPacketSerializer.h"
#include "inet/applications/mqtt/MqttTopicFilter.h"

namespace inet::mqttsn {

int MqttSnSession::allocateMessageId()
{
    for (int count = 0; count < 65535; count++) {
        int candidate = nextMessageId;
        nextMessageId = candidate == 65535 ? 1 : candidate + 1;
        if (!transactions.count(candidate)) return candidate;
    }
    return 0;
}

int MqttSnSession::findTopic(const std::string& name) const
{
    for (const auto& entry : topics) if (entry.second == name) return entry.first;
    return 0;
}

int MqttSnSession::allocateTopic(const std::string& name)
{
    if (int existing = findTopic(name)) return existing;
    // MQTT-SN v1.2 5.3.11 and 7.3: reserved ids, and per-client mappings.
    for (int count = 0; count < 65534; count++) {
        int candidate = nextTopicId;
        nextTopicId = candidate == 65534 ? 1 : candidate + 1;
        if (!topics.count(candidate)) { topics[candidate] = name; return candidate; }
    }
    return 0;
}

std::string MqttSnSession::resolveTopic(const MqttSnPacket& packet) const
{
    if (topicKind(packet) == SHORT_TOPIC) {
        std::string name;
        name.push_back(packet.getTopicId() >> 8); name.push_back(packet.getTopicId() & 255);
        return mqtt::MqttTopicFilter::isValidTopic(name) ? name : std::string();
    }
    const auto& table = topicKind(packet) == PREDEFINED_TOPIC ? predefinedTopics : topics;
    auto found = table.find(packet.getTopicId());
    return found == table.end() ? std::string() : found->second;
}

void MqttSnSession::clearExchanges()
{
    transactions.clear(); replies.clear(); inboundQos2.clear(); publications.clear(); qosZeroTopics.clear();
    publishAfter = SIMTIME_ZERO; nextMessageId = 1;
}

bool MqttSnSession::sameRequest(const MqttSnPacket& first, const MqttSnPacket& second)
{
    auto a = staticPtrCast<MqttSnPacket>(first.dupShared());
    auto b = staticPtrCast<MqttSnPacket>(second.dupShared());
    a->setFlags(a->getFlags() & ~DUPLICATE); b->setFlags(b->getFlags() & ~DUPLICATE);
    a->setExtendedLength(false); b->setExtendedLength(false);
    return MqttSnPacketSerializer::encode(*a) == MqttSnPacketSerializer::encode(*b);
}

Ptr<MqttSnPacket> MqttSnSession::makePacket(int type, int messageId)
{
    auto packet = makeShared<MqttSnPacket>(); packet->setType(type); packet->setMessageId(messageId);
    return packet;
}

std::vector<uint8_t> MqttSnSession::payload(const MqttSnPacket& packet)
{
    std::vector<uint8_t> result;
    for (size_t i = 0; i < packet.getPayloadArraySize(); i++) result.push_back(packet.getPayload(i));
    return result;
}

void MqttSnSession::setPayload(const Ptr<MqttSnPacket>& packet, const std::vector<uint8_t>& bytes)
{
    packet->setPayloadArraySize(bytes.size());
    for (size_t i = 0; i < bytes.size(); i++) packet->setPayload(i, bytes[i]);
}

std::vector<uint8_t> MqttSnSession::parseHex(const char *text)
{
    std::vector<uint8_t> result;
    int high = -1;
    for (unsigned char ch : std::string(text ? text : "")) {
        if (ch == ' ' || ch == '\n' || ch == '\t') continue;
        int digit = ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : -1;
        if (digit < 0) throw cRuntimeError("Invalid MQTT-SN hexadecimal payload");
        if (high < 0) high = digit;
        else { result.push_back(high * 16 + digit); high = -1; }
    }
    if (high >= 0) throw cRuntimeError("Odd-length MQTT-SN hexadecimal payload");
    return result;
}

void MqttSnSession::readPredefined(cXMLElement *configuration, std::map<int, std::string>& table)
{
    table.clear();
    for (auto element : configuration->getChildren()) {
        const char *idText = element->getAttribute("id"), *name = element->getAttribute("name");
        if (!idText || !name) throw cRuntimeError("Predefined MQTT-SN topic needs id/name");
        char *end = nullptr; long id = strtol(idText, &end, 10);
        if (*end || id < 1 || id > 65534 || !mqtt::MqttTopicFilter::isValidTopic(name) || !table.emplace(id, name).second)
            throw cRuntimeError("Invalid/duplicate predefined MQTT-SN topic");
    }
}

} // namespace inet::mqttsn
