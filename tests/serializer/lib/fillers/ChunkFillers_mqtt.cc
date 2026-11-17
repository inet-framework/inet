// SPDX-License-Identifier: LGPL-3.0-or-later
#include "../ChunkFillers.h"
#include "inet/features.h"
#ifdef INET_WITH_MQTT
#include "inet/applications/mqtt/MqttPacketSerializer.h"
#endif
namespace inet {
void addFillers_mqtt(std::vector<ChunkFiller>& fillers)
{
#ifdef INET_WITH_MQTT
    for (int type = mqtt::CONNECT; type <= mqtt::DISCONNECT; type++)
        fillers.push_back({"inet::mqtt::MqttPacket", std::to_string(type), [type](Chunk *chunk) {
            auto packet = check_and_cast<mqtt::MqttPacket *>(chunk);
            packet->setType(type);
            packet->setClientIdentifier("roundtrip");
            packet->setTopic("factory/temperature");
            packet->setPacketIdentifier(0x1234);
            if (type == mqtt::CONNECT) {
                packet->setCleanSession(false); packet->setKeepAlive(0x1234);
                packet->setWill(true); packet->setWillQos(2); packet->setWillRetain(true); packet->setWillTopic("will/state");
                packet->setWillPayloadArraySize(2); packet->setWillPayload(0,0); packet->setWillPayload(1,255);
                packet->setUserNamePresent(true); packet->setUserName("roundtripUser");
                packet->setPasswordPresent(true); packet->setPasswordArraySize(2); packet->setPassword(0,255); packet->setPassword(1,0);
            }
            if (type == mqtt::CONNACK) packet->setSessionPresent(true);
            if (type == mqtt::PUBLISH) {
                packet->setQos(2); packet->setDuplicate(true); packet->setRetain(true);
                packet->setPayloadArraySize(3); packet->setPayload(0,0); packet->setPayload(1,255); packet->setPayload(2,65);
            }
            if (type == mqtt::SUBSCRIBE || type == mqtt::UNSUBSCRIBE) {
                packet->setFiltersArraySize(1); packet->setFilters(0, "factory/+");
            }
            if (type == mqtt::SUBSCRIBE || type == mqtt::SUBACK) {
                packet->setResultsArraySize(1); packet->setResults(0, 2);
            }
            packet->setChunkLength(B(mqtt::MqttPacketSerializer::encode(*packet).size()));
        }});
    fillers.push_back({"inet::mqtt::MqttPacket", "refused", [](Chunk *chunk) {
        auto packet = check_and_cast<mqtt::MqttPacket *>(chunk);
        packet->setType(mqtt::CONNACK); packet->setReturnCode(4);
        packet->setChunkLength(B(mqtt::MqttPacketSerializer::encode(*packet).size()));
    }});
#endif
}
}
