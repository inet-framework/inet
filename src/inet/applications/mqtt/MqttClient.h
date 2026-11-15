// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTCLIENT_H
#define __INET_MQTTCLIENT_H
#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/mqtt/MqttConnection.h"
#include "inet/applications/mqtt/MqttSession.h"

namespace inet::mqtt {
class INET_API MqttClient : public ApplicationBase, public TcpSocket::ICallback
{
  protected:
    struct Action {
        simtime_t offset;
        Ptr<MqttPacket> packet;
    };
    TcpSocket socket;
    MqttConnection connection;
    MqttSession session;
    std::vector<Action> actions;
    size_t actionIndex = 0;
    Ptr<MqttPacket> pendingSubscription;
    simtime_t workloadOrigin = -1;
    cMessage *connectTimer = nullptr;
    cMessage *timeoutTimer = nullptr;
    cMessage *workTimer = nullptr;

    void initialize(int stage) override;
    int numInitStages() const override { return NUM_INIT_STAGES; }
    void handleMessageWhenUp(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *operation) override;
    void handleStopOperation(LifecycleOperation *operation) override;
    void handleCrashOperation(LifecycleOperation *operation) override;
    void readWorkload();
    void connect();
    void disconnect();
    void scheduleWork();
    void runWork();
    void sendControl(const Ptr<MqttPacket>& packet);
    void receiveControl(const Ptr<MqttPacket>& packet);
    void failConnection();

    void socketEstablished(TcpSocket *socket) override;
    void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    void socketAvailable(TcpSocket *, TcpAvailableInfo *) override;
    void socketPeerClosed(TcpSocket *socket) override;
    void socketClosed(TcpSocket *socket) override;
    void socketFailure(TcpSocket *socket, int code) override;
    void socketStatusArrived(TcpSocket *, TcpStatusInfo *status) override { delete status; }
    void socketDeleted(TcpSocket *) override {}

  public:
    ~MqttClient();
};
}
#endif
