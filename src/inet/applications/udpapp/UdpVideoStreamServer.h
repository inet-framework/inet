//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Based on the video streaming app of the similar name by Johnny Lai.
//

#ifndef __INET_UDPVIDEOSTREAMSERVER_H
#define __INET_UDPVIDEOSTREAMSERVER_H

#include <map>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/packet/Packet.h"
#include "inet/queueing/contract/IPassivePacketSink.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

using namespace inet::queueing;

/**
 * Stream VBR video streams to clients.
 *
 * Cooperates with UdpVideoStreamClient. UdpVideoStreamClient requests a stream
 * and UdpVideoStreamServer starts streaming to them. Capable of handling
 * streaming to multiple clients.
 */
class INET_API UdpVideoStreamServer : public ApplicationBase, public UdpSocket::ICallback, public IPassivePacketSink, public IModuleInterfaceLookup
{
  public:
    struct VideoStreamData {
        cMessage *timer = nullptr; // self timer msg
        L3Address clientAddr; // client address
        int clientPort = -1; // client UDP port
        long videoSize = 0; // total size of video
        long bytesLeft = 0; // bytes left to transmit
        long numPkSent = 0; // number of packets sent
    };

  protected:
    typedef std::map<long int, VideoStreamData> VideoStreamMap;

    // state
    VideoStreamMap streams;
    UdpSocket socket;

    // parameters
    int localPort = -1;
    cPar *sendInterval = nullptr;
    cPar *packetLen = nullptr;
    cPar *videoSize = nullptr;

    // statistics
    unsigned int numStreams = 0; // number of video streams served
    unsigned long numPkSent = 0; // total number of packets sent
    static simsignal_t reqStreamBytesSignal; // length of video streams served

    virtual void processStreamRequest(Packet *msg);
    virtual void sendStreamData(cMessage *timer);

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void clearStreams();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

  public:
    UdpVideoStreamServer() {}
    virtual ~UdpVideoStreamServer();

    virtual bool canPushSomePacket(const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual bool canPushPacket(Packet *packet, const cGate *gate) const override { return gate->isName("socketIn"); }
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
    virtual void pushPacketStart(Packet *packet, const cGate *gate, bps datarate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketEnd(Packet *packet, const cGate *gate) override { throw cRuntimeError("TODO"); }
    virtual void pushPacketProgress(Packet *packet, const cGate *gate, bps datarate, b position, b extraProcessableLength = b(0)) override { throw cRuntimeError("TODO"); }

    virtual cGate *lookupModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction) override;
};

} // namespace inet

#endif

