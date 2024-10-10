//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPSERVER_H
#define __INET_SCTPSERVER_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

using namespace inet::queueing;

/**
 * Implements the SctpServer simple module. See the NED file for more info.
 */
class INET_API SctpServer : public cSimpleModule, public SctpSocket::ICallback, public LifecycleUnsupported, public IPassivePacketSink, public IModuleInterfaceLookup
{
  protected:
    struct ServerAssocStat {
        SctpSocket *socket = nullptr;
        simtime_t start;
        simtime_t stop;
        simtime_t lifeTime;
        unsigned long int rcvdBytes;
        unsigned long int sentPackets;
        unsigned long int rcvdPackets;
        bool abortSent;
        bool peerClosed;
    };
    typedef std::map<int, ServerAssocStat> ServerAssocStatMap;
    typedef std::map<int, cOutVector *> BytesPerAssoc;
    typedef std::map<int, cOutVector *> EndToEndDelay;

    // parameters
    int inboundStreams;
    int outboundStreams;
    int queueSize;
    double delay;
    double delayFirstRead;
    bool finishEndsSimulation;
    bool echo;
    bool ordered;

    // state
    SctpSocket *socket;
    cMessage *timeoutMsg;
    cMessage *delayTimer;
    cMessage *delayFirstReadTimer;
    int lastStream;
    int assocId;
    bool readInt;
    bool schedule;
    bool firstData;
    bool shutdownReceived;
    bool abortSent;
    EndToEndDelay endToEndDelay;

    // statistics
    int numSessions;
    int count;
    int notificationsReceived;
    unsigned long int bytesSent;
    unsigned long int packetsSent;
    unsigned long int packetsRcvd;
    unsigned long int numRequestsToSend; // requests to send in this session
    BytesPerAssoc bytesPerAssoc;
    ServerAssocStatMap serverAssocStatMap;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    void handleTimer(cMessage *msg);

    void generateAndSend();

  public:
    virtual ~SctpServer();
    SctpServer();

    virtual void socketDataArrived(SctpSocket *socket, Packet *packet, bool urgent) override;
    virtual void socketDataNotificationArrived(SctpSocket *socket, Message *msg) override;
    virtual void socketAvailable(SctpSocket *socket, Indication *indication) override;
    virtual void socketEstablished(SctpSocket *socket, Indication *indication) override;

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

