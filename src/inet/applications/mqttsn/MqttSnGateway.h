// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSNGATEWAY_H
#define __INET_MQTTSNGATEWAY_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/mqttsn/MqttSnSession.h"
#include "inet/applications/mqtt/MqttConnection.h"
#include "inet/applications/mqtt/MqttSession.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet::mqttsn {

class INET_API MqttSnGateway : public ApplicationBase, public UdpSocket::ICallback, public TcpSocket::ICallback
{
  protected:
    // Stable sessionStateChanged signal values.
    enum Phase {
        DISCONNECTED = 0, WILL_TOPIC = 1, WILL_MESSAGE = 2, CONNECTING = 3,
        ACTIVE = 4, ASLEEP = 5, AWAKE = 6, LOST = 7
    };
    struct Peer : MqttSnSession {
        L3Address address;
        int port = 0;
        std::string clientIdentifier;
        Phase phase = DISCONNECTED;
        bool cleanSession = true;
        int keepAlive = 0;
        int sleepDuration = 0;
        int socketId = -1;
        Ptr<MqttSnPacket> will;
        Ptr<const MqttSnPacket> connectRequest;
        mqtt::MqttSession mqttSession;
        std::map<int, Ptr<const MqttSnPacket>> pendingSubscriptions;
        std::map<int, std::pair<std::string, int>> restoring;
        std::deque<std::pair<std::string, int>> restoreQueue;
        simtime_t deadline = -1;
        simtime_t brokerDeadline = -1;
        simtime_t brokerLastSent = SIMTIME_ZERO;
        simtime_t brokerPingDeadline = -1;
        bool brokerActive = false;
    };
    UdpSocket socket;
    SocketMap brokerSockets;
    std::map<std::string, Peer> peers;
    Peer shared;
    cMessage *timer = nullptr;
    bool bound = false;
    simtime_t sharedReconnectAt = SIMTIME_ZERO;
    simtime_t advertiseAt = SIMTIME_ZERO;

    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessageWhenUp(cMessage *message) override;
    void handleMessageWhenDown(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *) override;
    void handleStopOperation(LifecycleOperation *) override;
    void handleCrashOperation(LifecycleOperation *) override;
    void handleActiveOperationTimeout(cMessage *) override;
    void sendTo(const Ptr<MqttSnPacket>& packet, const L3Address& address, int port);
    void transmit(Peer& peer, const Ptr<MqttSnPacket>& packet, bool track = false);
    void receiveControlPacket(Peer& peer, const Ptr<MqttSnPacket>& packet);
    void acceptConnect(const Ptr<MqttSnPacket>& packet, const L3Address& source, int port);
    void openBroker(Peer& peer);
    void closeBroker(Peer& peer);
    void failBroker(Peer& peer, int refusal = CONGESTION);
    void brokerSend(Peer& peer, const Ptr<mqtt::MqttPacket>& packet);
    void brokerReceive(Peer& peer, const Ptr<mqtt::MqttPacket>& packet);
    void restoreSubscriptions(Peer& peer);
    Peer *findPeer(TcpSocket *socket);
    void enqueueBroker(Peer& peer, const Ptr<MqttSnPacket>& packet, const std::string& topic);
    void finishSetup(Peer& peer);
    void disconnectPeer(Peer& peer, bool lost);
    void drive(Peer& peer);
    void scheduleTimer();
    void reapSockets();
    void rememberReply(Peer& peer, const Ptr<MqttSnPacket>& request, const Ptr<MqttSnPacket>& reply);
    bool replayReply(Peer& peer, const Ptr<MqttSnPacket>& request);
    void socketDataArrived(UdpSocket *, Packet *packet) override;
    void socketErrorArrived(UdpSocket *, Indication *indication) override;
    void socketClosed(UdpSocket *) override;
    void socketEstablished(TcpSocket *socket) override;
    void socketDataArrived(TcpSocket *socket, Packet *packet, bool urgent) override;
    void socketPeerClosed(TcpSocket *socket) override;
    void socketClosed(TcpSocket *socket) override;
    void socketFailure(TcpSocket *socket, int code) override;
    void socketAvailable(TcpSocket *, TcpAvailableInfo *info) override { delete info; }
    void socketStatusArrived(TcpSocket *, TcpStatusInfo *status) override { delete status; }
    void socketDeleted(TcpSocket *) override {}

  public:
    ~MqttSnGateway();
};

} // namespace inet::mqttsn
#endif
