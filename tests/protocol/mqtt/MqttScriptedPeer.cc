// SPDX-License-Identifier: LGPL-3.0-or-later
#include "MqttScriptedPeer.h"
namespace inet::mqtttest {
Define_Module(MqttScriptedPeer);
void MqttScriptedPeer::sendBytes(TcpSocket& socket, const std::vector<uint8_t>& bytes)
{
    Enter_Method("sendBytes");
    socket.send(new Packet("mqttBytes", makeShared<BytesChunk>(bytes)));
}
void MqttScriptedPeer::sendMqtt(TcpSocket& socket, const Ptr<mqtt::MqttPacket>& packet)
{
    sendBytes(socket, mqtt::MqttPacketSerializer::encode(*packet));
}
void MqttScriptedPeer::socketDataArrived(TcpSocket *socket, Packet *packet, bool)
{
    socket->getReadBuffer()->push(packet->peekData());
    delete packet;
    while (auto decoded = mqtt::MqttConnection::extract(*socket->getReadBuffer())) {
        received.push_back(decoded);
        Packet observation("mqttReceived", decoded);
        emit(registerSignal("packetReceivedFromLower"), &observation);
        if (responder) responder(*this, *socket, decoded);
    }
}
}
