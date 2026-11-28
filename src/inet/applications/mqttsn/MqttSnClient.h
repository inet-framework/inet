// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_MQTTSNCLIENT_H
#define __INET_MQTTSNCLIENT_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/applications/mqttsn/MqttSnSession.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet::mqttsn {

class INET_API MqttSnClient : public ApplicationBase, public UdpSocket::ICallback
{
  protected:
    // Stable sessionStateChanged signal values.
    enum Phase { DISCONNECTED = 0, CONNECTING = 1, ACTIVE = 2, ASLEEP = 3, AWAKE = 4, CLOSING = 5 };
    struct Candidate { L3Address address; simtime_t expiry; };
    UdpSocket socket;
    MqttSnSession session;
    Phase phase = DISCONNECTED;
    L3Address gateway;
    std::map<int, Candidate> candidates;
    std::set<int> attemptedGateways;
    std::vector<std::pair<simtime_t, Ptr<MqttSnPacket>>> actions;
    std::deque<Ptr<MqttSnPacket>> reconciliation;
    size_t actionIndex = 0;
    cMessage *timer = nullptr;
    simtime_t origin = -1;
    simtime_t reconnectAt = SIMTIME_ZERO;
    simtime_t lastSent = SIMTIME_ZERO;
    simtime_t searchAt = -1;
    simtime_t searchResponseAt = -1;
    simtime_t gatewayInfoAt = -1;
    simtime_t searchBackoff = SIMTIME_ZERO;
    int selectedGatewayId = -1;
    bool resetSession = false;
    bool effectiveClean = true;
    bool bound = false;
    bool reconcileOnConnect = false;

    int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessageWhenUp(cMessage *message) override;
    void handleMessageWhenDown(cMessage *message) override;
    void handleStartOperation(LifecycleOperation *) override;
    void handleStopOperation(LifecycleOperation *) override;
    void handleCrashOperation(LifecycleOperation *) override;
    void handleActiveOperationTimeout(cMessage *) override;
    void scheduleTimer();
    void drive();
    void connectGateway();
    void loseGateway();
    void changePhase(Phase value);
    void transmit(const Ptr<MqttSnPacket>& packet, bool track = false);
    void sendTo(const Ptr<MqttSnPacket>& packet, const L3Address& address, int port);
    void receiveControlPacket(const Ptr<MqttSnPacket>& packet, const L3Address& source, int port);
    void receiveDiscovery(const Ptr<MqttSnPacket>& packet, const L3Address& source);
    void readWorkload();
    void complete(int key);
    void deliver(const Ptr<MqttSnPacket>& packet);
    void socketDataArrived(UdpSocket *, Packet *packet) override;
    void socketErrorArrived(UdpSocket *, Indication *indication) override;
    void socketClosed(UdpSocket *) override;

  public:
    ~MqttSnClient();
};

} // namespace inet::mqttsn
#endif
