// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSNSESSION_H
#define __INET_MQTTSNSESSION_H

#include <deque>
#include <set>
#include "inet/applications/mqttsn/MqttSnProtocol.h"

namespace inet::mqttsn {

struct INET_API MqttSnTransaction
{
    Ptr<const MqttSnPacket> packet;
    simtime_t deadline = SIMTIME_ZERO;
    int retries = 0;
};

// No scheduled event or socket lives in a session. Their module owns that lifetime.
struct INET_API MqttSnSession
{
    std::map<int, std::string> topics;
    std::map<int, std::string> predefinedTopics;
    std::map<std::string, int> subscriptions;
    std::set<std::string> excludedTopics;
    std::set<int> inboundQos2;
    std::map<int, int> qosZeroTopics; // Latest representation sent with each feedback TopicId.
    std::map<int, MqttSnTransaction> transactions;
    std::map<int, std::pair<Ptr<const MqttSnPacket>, Ptr<const MqttSnPacket>>> replies;
    std::deque<Ptr<MqttSnPacket>> publications;
    int nextMessageId = 1;
    int nextTopicId = 1;
    simtime_t publishAfter = SIMTIME_ZERO;

    int allocateMessageId();
    int allocateTopic(const std::string& name);
    int findTopic(const std::string& name) const;
    std::string resolveTopic(const MqttSnPacket& packet) const;
    void clearExchanges();
    static bool sameRequest(const MqttSnPacket& first, const MqttSnPacket& second);
    static Ptr<MqttSnPacket> makePacket(int type, int messageId = 0);
    static std::vector<uint8_t> payload(const MqttSnPacket& packet);
    static void setPayload(const Ptr<MqttSnPacket>& packet, const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> parseHex(const char *text);
    static void readPredefined(cXMLElement *configuration, std::map<int, std::string>& topics);
};

} // namespace inet::mqttsn
#endif
