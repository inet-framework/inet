//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_COAPTESTPEER_H
#define __INET_PROTOCOLTEST_COAPTESTPEER_H

#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {
namespace protocoltest {

class INET_API CoapTestPeer : public ApplicationBase, public UdpSocket::ICallback
{
  public:
    struct ReceivedDatagram
    {
        simtime_t arrivalTime = SIMTIME_ZERO;
        L3Address srcAddr;
        int srcPort = -1;
        L3Address destAddr;
        int destPort = -1;
        std::vector<uint8_t> bytes;
    };

    struct ScheduledSend
    {
        simtime_t time = SIMTIME_ZERO;
        std::string destAddrStr;
        int destPort = 5683;
        std::vector<uint8_t> bytes;
        std::string packetName = "PeerRawDatagram";
    };

    using InboundCallback = std::function<void(CoapTestPeer&, const ReceivedDatagram&)>;

  protected:
    UdpSocket socket;
    int localPort = 5683;
    cMessage *sendTimer = nullptr;
    std::deque<ScheduledSend> scheduledSends;
    std::vector<ReceivedDatagram> receivedDatagrams;
    InboundCallback inboundCallback;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    void scheduleNextTimer();

  public:
    CoapTestPeer() = default;
    virtual ~CoapTestPeer();

    void sendRawNow(const L3Address& destAddr, int destPort, const std::vector<uint8_t>& bytes,
            const char *packetName = "PeerRawDatagram");
    void sendRawNow(const std::string& destAddrStr, int destPort, const std::vector<uint8_t>& bytes,
            const char *packetName = "PeerRawDatagram");
    void scheduleRawSend(simtime_t atTime, const std::string& destAddrStr, int destPort,
            const std::vector<uint8_t>& bytes, const char *packetName = "PeerRawDatagram");
    void setInboundCallback(InboundCallback cb) { inboundCallback = std::move(cb); }

    const std::vector<ReceivedDatagram>& getReceivedDatagrams() const { return receivedDatagrams; }
    void clearReceivedDatagrams() { receivedDatagrams.clear(); }
};

} // namespace protocoltest
} // namespace inet

#endif
