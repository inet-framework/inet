// SPDX-License-Identifier: LGPL-3.0-or-later
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

namespace inet::mqttsn;

enum MqttSnPacketType
{
    ADVERTISE = 0;
    SEARCHGW = 1;
    GWINFO = 2;
    CONNECT = 4;
    CONNACK = 5;
    WILLTOPICREQ = 6;
    WILLTOPIC = 7;
    WILLMSGREQ = 8;
    WILLMSG = 9;
    REGISTER = 10;
    REGACK = 11;
    PUBLISH = 12;
    PUBACK = 13;
    PUBCOMP = 14;
    PUBREC = 15;
    PUBREL = 16;
    SUBSCRIBE = 18;
    SUBACK = 19;
    UNSUBSCRIBE = 20;
    UNSUBACK = 21;
    PINGREQ = 22;
    PINGRESP = 23;
    DISCONNECT = 24;
    WILLTOPICUPD = 26;
    WILLTOPICRESP = 27;
    WILLMSGUPD = 28;
    WILLMSGRESP = 29;
}

// One complete v1.2 message. Only the fields of its message type are serialized.
// Raw flags and length form are retained for lossless decoding/inspection.
class MqttSnPacket extends FieldsChunk
{
    int type @enum(MqttSnPacketType) = PINGRESP;
    bool extendedLength = false;
    int flags = 0;
    int protocolId = 1;
    int duration = 0; // integer seconds on the wire
    bool durationPresent = false;
    bool willTopicPresent = false; // absent means the two-octet delete form
    int gatewayId = 0;
    int radius = 1;
    int topicId = 0;
    int messageId = 0;
    int returnCode = 0;
    string clientIdentifier;
    string topic;
    uint8_t payload[];
    uint8_t gatewayAddress[];
}
