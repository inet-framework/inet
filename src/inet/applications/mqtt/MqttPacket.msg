// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::mqtt;

enum MqttPacketType
{
    CONNECT = 1;
    CONNACK = 2;
    PUBLISH = 3;
    PUBACK = 4;
    PUBREC = 5;
    PUBREL = 6;
    PUBCOMP = 7;
    SUBSCRIBE = 8;
    SUBACK = 9;
    UNSUBSCRIBE = 10;
    UNSUBACK = 11;
    PINGREQ = 12;
    PINGRESP = 13;
    DISCONNECT = 14;
}

// One complete MQTT 3.1.1 control packet, including its opaque binary payload.
class MqttPacket extends FieldsChunk
{
    int type @enum(MqttPacketType) = PINGREQ;
    bool duplicate = false;
    int qos = 0;
    bool retain = false;
    int packetIdentifier = 0;
    string protocolName = "MQTT";
    int protocolLevel = 4;
    string clientIdentifier;
    bool cleanSession = true;
    int keepAlive = 0; // seconds, unsigned 16-bit wire value
    bool will = false;
    int willQos = 0;
    bool willRetain = false;
    string willTopic;
    uint8_t willPayload[];
    bool userNamePresent = false;
    string userName;
    bool passwordPresent = false;
    uint8_t password[];
    bool sessionPresent = false;
    int returnCode = 0;
    string topic;
    uint8_t payload[];
    string filters[];
    int results[]; // requested QoS in SUBSCRIBE, return codes in SUBACK
}
