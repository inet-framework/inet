//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_SCTPNATPEER_H
#define __INET_SCTPNATPEER_H

#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

struct nat_message
{

    uint16_t multi;
    uint16_t reserved = 0;
    uint16_t peer1;
    uint16_t peer2;
    uint16_t portPeer1;
    uint16_t portPeer2;
    uint16_t numAddrPeer1;
    uint16_t numAddrPeer2;
    uint32_t peer1Addresses[0];
    uint32_t peer2Addresses[0];
};

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */

class INET_API SctpNatPeer : public cSimpleModule, public SctpSocket::ICallback, public LifecycleUnsupported
{
  protected:
//    SctpAssociation* assoc;
    int32_t notifications;
    int32_t serverAssocId;
    SctpSocket clientSocket;
    SctpSocket peerSocket;
    SctpSocket rendezvousSocket;
    double delay;
    bool echo;
    bool schedule;
    bool shutdownReceived;
//    long bytesRcvd;
    int64_t bytesSent;
    int32_t packetsSent;
    int32_t packetsRcvd;
    int32_t numSessions;
    int32_t numRequestsToSend; // requests to send in this session
    bool ordered;
    int32_t queueSize;
    cMessage *timeoutMsg;
    cMessage *timeMsg;
    int32_t outboundStreams;
    int32_t inboundStreams;
    int32_t bytesRcvd;
    int32_t echoedBytesSent;
    int32_t lastStream;
    bool sendAllowed;
    int32_t chunksAbandoned;
    int32_t numPacketsToReceive;
    bool rendezvous;
    L3Address peerAddress;
    int32_t peerPort;
    AddressVector peerAddressList;
    AddressVector localAddressList;
//    cOutVector* rcvdBytes;
    typedef std::map<int32_t, int64_t> RcvdPacketsPerAssoc;
    RcvdPacketsPerAssoc rcvdPacketsPerAssoc;
    typedef std::map<int32_t, int64_t> SentPacketsPerAssoc;
    SentPacketsPerAssoc sentPacketsPerAssoc;
    typedef std::map<int32_t, int64_t> RcvdBytesPerAssoc;
    RcvdBytesPerAssoc rcvdBytesPerAssoc;
    typedef std::map<int32_t, cOutVector *> BytesPerAssoc;
    BytesPerAssoc bytesPerAssoc;
    typedef std::map<int32_t, cHistogram *> HistEndToEndDelay;
    HistEndToEndDelay histEndToEndDelay;
    typedef std::map<int32_t, cOutVector *> EndToEndDelay;
    EndToEndDelay endToEndDelay;
    void sendOrSchedule(cMessage *msg);
    void sendRequest(bool last = true);

  public:
    SctpNatPeer();
    virtual ~SctpNatPeer();
    struct pathStatus {
        bool active;
        bool primaryPath;
        L3Address pid;
    };
    typedef std::map<L3Address, pathStatus> SctpPathStatus;
    SctpPathStatus sctpPathStatus;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    void initialize(int stage) override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void handleTimer(cMessage *msg);
//    void setAssociation(SctpAssociation *_assoc) { assoc = _assoc; }
    void generateAndSend();
    void connect(L3Address connectAddress, int32_t connectPort);
    void connectx(AddressVector connectAddressList, int32_t connectPort);

    virtual void socketAvailable(SctpSocket *socket, Indication *indication) override {
        throw cRuntimeError("Model error, this module doesn't use any listener SCTP sockets");
    }

    /** Does nothing but update statistics/status. Redefine to perform or schedule first sending. */
    void socketEstablished(SctpSocket *socket, unsigned long int buffer) override;

    /**
     * Does nothing but update statistics/status. Redefine to perform or schedule next sending.
     * Beware: this funcion deletes the incoming message, which might not be what you want.
     */
    void socketDataArrived(SctpSocket *socket, Packet *msg, bool urgent) override;

    void socketDataNotificationArrived(SctpSocket *socket, Message *msg) override;
    /** Since remote SCTP closed, invokes close(). Redefine if you want to do something else. */
    void socketPeerClosed(SctpSocket *socket) override;

    /** Does nothing but update statistics/status. Redefine if you want to do something else, such as opening a new connection. */
    void socketClosed(SctpSocket *socket) override;

    /** Does nothing but update statistics/status. Redefine if you want to try reconnecting after a delay. */
    void socketFailure(SctpSocket *socket, int32_t code) override;

    /** Redefine to handle incoming SctpStatusInfo. */
    void socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) override;
    //@}
    void msgAbandonedArrived(SctpSocket *socket) override;
    // void setAssociation(SctpAssociation *_assoc) {assoc = _assoc; }

    void setPrimaryPath();
    void sendStreamResetNotification();
    void sendRequestArrived(SctpSocket *socket) override;
    void sendQueueRequest();
    void shutdownReceivedArrived(SctpSocket *socket) override;
    void sendqueueFullArrived(SctpSocket *socket) override;
    void addressAddedArrived(SctpSocket *socket, L3Address localAddr, L3Address remoteAddr) override;
    void setStatusString(const char *s);
};

} // namespace inet

#endif

