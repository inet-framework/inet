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
            if (type == mqtt::SUBSCRIBE || type == mqtt::UNSUBSCRIBE) {
                packet->setFiltersArraySize(1); packet->setFilters(0, "factory/+");
            }
            if (type == mqtt::SUBSCRIBE || type == mqtt::SUBACK) {
                packet->setResultsArraySize(1); packet->setResults(0, 2);
            }
            packet->setChunkLength(B(mqtt::MqttPacketSerializer::encode(*packet).size()));
        }});
#endif
}
}
