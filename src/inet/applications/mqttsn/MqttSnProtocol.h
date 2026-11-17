// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSNPROTOCOL_H
#define __INET_MQTTSNPROTOCOL_H

#include "inet/common/Protocol.h"
#include "inet/applications/mqttsn/MqttSnPacket_m.h"

namespace inet::mqttsn {

extern INET_API const Protocol mqttSnProtocol;

enum MqttSnFlag { DUPLICATE = 0x80, RETAIN = 0x10, WILL = 0x08, CLEAN_SESSION = 0x04 };
enum MqttSnTopicKind { NORMAL_TOPIC = 0, PREDEFINED_TOPIC = 1, SHORT_TOPIC = 2 };
enum MqttSnReturnCode { ACCEPTED = 0, CONGESTION = 1, INVALID_TOPIC = 2, NOT_SUPPORTED = 3 };

inline int qos(const MqttSnPacket& packet) { int value = (packet.getFlags() >> 5) & 3; return value == 3 ? -1 : value; }
inline int topicKind(const MqttSnPacket& packet) { return packet.getFlags() & 3; }
inline int qosFlags(int value) { return (value == -1 ? 3 : value) << 5; }

} // namespace inet::mqttsn
#endif
