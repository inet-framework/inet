//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTPPEER_H
#define __INET_SCTPPEER_H

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

class SctpConnectInfo;

/**
 * Implements the SctpPeer simple module. See the NED file for more info.
 */
class INET_API SctpPeer : public cSimpleModule, public SctpSocket::ICallback, public LifecycleUnsupported
{
  protected:
    struct PathStatus {
        bool active;
        bool primaryPath;
        Ipv4Address pid;
    };
    typedef std::map<int, long> RcvdPacketsPerAssoc;
    typedef std::map<int, long> SentPacketsPerAssoc;
    typedef std::map<int, long> RcvdBytesPerAssoc;
    typedef std::map<int, cOutVector *> BytesPerAssoc;
    typedef std::map<int, cHistogram *> HistEndToEndDelay;
    typedef std::map<int, cOutVector *> EndToEndDelay;
    typedef std::map<L3Address, PathStatus> SctpPathStatus;

    // parameters
    double delay;
    bool echo;
    bool ordered;
    bool schedule;
    int queueSize;
    int outboundStreams;
    int inboundStreams;

    // state
    SctpPathStatus sctpPathStatus;
    SctpSocket clientSocket;
    SctpSocket listeningSocket;
    cMessage *timeoutMsg;
    cMessage *timeMsg;
    cMessage *connectTimer;
    bool shutdownReceived;
    bool sendAllowed;
    int serverAssocId;
    int numRequestsToSend; // requests to send in this session
    int lastStream;
    int numPacketsToReceive;

    // statistics
    RcvdPacketsPerAssoc rcvdPacketsPerAssoc;
    SentPacketsPerAssoc sentPacketsPerAssoc;
    RcvdBytesPerAssoc rcvdBytesPerAssoc;
    BytesPerAssoc bytesPerAssoc;
    HistEndToEndDelay histEndToEndDelay;
    EndToEndDelay endToEndDelay;
    long bytesSent;
    int echoedBytesSent;
    int packetsSent;
    int bytesRcvd;
    int packetsRcvd;
    int notificationsReceived;
    int numSessions;
    int chunksAbandoned;
    static simsignal_t echoedPkSignal;

  protected:

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    void handleTimer(cMessage *msg);

    void connect();
    virtual void socketAvailable(SctpSocket *socket, Indication *indication) override { throw cRuntimeError("Model error, this module doesn't use any listener SCTP sockets"); }
    void socketEstablished(SctpSocket *socket, unsigned long int buffer) override;
    void socketDataArrived(SctpSocket *socket, Packet *msg, bool urgent) override;
    void socketDataNotificationArrived(SctpSocket *socket, Message *msg) override;
    void socketPeerClosed(SctpSocket *socket) override;
    void socketClosed(SctpSocket *socket) override;
    void socketFailure(SctpSocket *socket, int code) override;

    /* Redefine to handle incoming SctpStatusInfo */
    void socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) override;

    void sendRequest(bool last = true);
    void sendOrSchedule(cMessage *msg);
    void generateAndSend();
    void sendRequestArrived(SctpSocket *socket) override;
    void sendQueueRequest();
    void shutdownReceivedArrived(SctpSocket *socket) override;
    void sendqueueFullArrived(SctpSocket *socket) override;
    void msgAbandonedArrived(SctpSocket *socket) override;
    void setStatusString(const char *s);

  public:
    SctpPeer();
    ~SctpPeer();
};

} // namespace inet

#endif

