// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTBROKER_H
#define __INET_MQTTBROKER_H
#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/mqtt/MqttConnection.h"
#include "inet/applications/mqtt/MqttSession.h"
#include "inet/common/socket/SocketMap.h"

namespace inet::mqtt {
class INET_API MqttBroker : public ApplicationBase, public TcpSocket::ICallback
{
  protected:
    TcpSocket listener;
    SocketMap socketMap;
    std::map<int, MqttConnection> connections;
    std::map<std::string, MqttSession> sessions;
    std::map<std::string, Ptr<const MqttPacket>> retained;
    cMessage *timeoutTimer = nullptr;

    void initialize(int stage) override;
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void handleMessageWhenUp(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    void handleActiveOperationTimeout(cMessage *message) override;
    void handleMessageWhenDown(cMessage *message) override;
    void reapClosed();
    void checkStopComplete();
    void sendControl(TcpSocket *socket, const Ptr<MqttPacket>& packet);
    void receiveControl(TcpSocket *socket, const Ptr<MqttPacket>& packet);
    void closeConnection(TcpSocket *socket);
    void scheduleTimeout();
    void route(const Ptr<const MqttPacket>& packet);

    void socketAvailable(TcpSocket *socket, TcpAvailableInfo *info) override;
    void socketEstablished(TcpSocket *) override {}
    void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    void socketPeerClosed(TcpSocket *socket) override;
    void socketClosed(TcpSocket *socket) override;
    void socketFailure(TcpSocket *socket, int code) override;
    void socketStatusArrived(TcpSocket *, TcpStatusInfo *status) override { delete status; }
    void socketDeleted(TcpSocket *) override {}

  public:
    ~MqttBroker();
};
}
#endif
