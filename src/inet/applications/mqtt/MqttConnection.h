// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTCONNECTION_H
#define __INET_MQTTCONNECTION_H

#include "inet/applications/mqtt/MqttPacketSerializer.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"

namespace inet::mqtt {

struct INET_API MqttConnection
{
    enum Phase { WAITING, ACTIVE, CLOSING };
    Phase phase = WAITING;
    std::string clientIdentifier;
    bool cleanSession = true;
    Ptr<const MqttPacket> will;
    simtime_t deadline = SIMTIME_ZERO;
    simtime_t lastReceived = SIMTIME_ZERO;
    int keepAlive = 0;

    static Ptr<MqttPacket> extract(ChunkQueue& queue);
};

} // namespace inet::mqtt
#endif
