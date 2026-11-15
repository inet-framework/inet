// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSCRIPTEDPEER_H
#define __INET_MQTTSCRIPTEDPEER_H
#include "TcpTestApp.h"
#include "inet/applications/mqtt/MqttConnection.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include <functional>

namespace inet::mqtttest {
class MqttScriptedPeer : public protocoltest::TcpTestApp
{
  protected:
    void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
  public:
    std::vector<Ptr<const mqtt::MqttPacket>> received;
    std::function<void(MqttScriptedPeer&, TcpSocket&, const Ptr<mqtt::MqttPacket>&)> responder;
    void sendBytes(TcpSocket& socket, const std::vector<uint8_t>& bytes);
    void sendMqtt(TcpSocket& socket, const Ptr<mqtt::MqttPacket>& packet);
};
}
#endif
