//
// Copyright 2004 Andras Varga
//
// This library is free software, you can redistribute it and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_SCTPNATPEER_H
#define __INET_SCTPNATPEER_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/LifecycleUnsupported.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"

namespace inet {

struct nat_message
{

    uint16      multi;
    uint16      reserved = 0;
    uint16      peer1;
    uint16      peer2;
    uint16      portPeer1;
    uint16      portPeer2;
    uint16      numAddrPeer1;
    uint16      numAddrPeer2;
    uint32_t    peer1Addresses[0];
    uint32_t    peer2Addresses[0];
};

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */

class INET_API SctpNatPeer : public cSimpleModule, public SctpSocket::ICallback, public LifecycleUnsupported
{
  protected:
    //SctpAssociation* assoc;
    int32 notifications;
    int32 serverAssocId;
    SctpSocket clientSocket;
    SctpSocket peerSocket;
    SctpSocket rendezvousSocket;
    double delay;
    bool echo;
    bool schedule;
    bool shutdownReceived;
    //long bytesRcvd;
    int64 bytesSent;
    int32 packetsSent;
    int32 packetsRcvd;
    int32 numSessions;
    int32 numRequestsToSend;    // requests to send in this session
    bool ordered;
    int32 queueSize;
    cMessage *timeoutMsg;
    cMessage *timeMsg;
    int32 outboundStreams;
    int32 inboundStreams;
    int32 bytesRcvd;
    int32 echoedBytesSent;
    int32 lastStream;
    bool sendAllowed;
    int32 chunksAbandoned;
    int32 numPacketsToReceive;
    bool rendezvous;
    L3Address peerAddress;
    int32 peerPort;
    AddressVector peerAddressList;
    AddressVector localAddressList;
    //cOutVector* rcvdBytes;
    typedef std::map<int32, int64> RcvdPacketsPerAssoc;
    RcvdPacketsPerAssoc rcvdPacketsPerAssoc;
    typedef std::map<int32, int64> SentPacketsPerAssoc;
    SentPacketsPerAssoc sentPacketsPerAssoc;
    typedef std::map<int32, int64> RcvdBytesPerAssoc;
    RcvdBytesPerAssoc rcvdBytesPerAssoc;
    typedef std::map<int32, cOutVector *> BytesPerAssoc;
    BytesPerAssoc bytesPerAssoc;
    typedef std::map<int32, cHistogram *> HistEndToEndDelay;
    HistEndToEndDelay histEndToEndDelay;
    typedef std::map<int32, cOutVector *> EndToEndDelay;
    EndToEndDelay endToEndDelay;
    void sendOrSchedule(cMessage *msg);
    void sendRequest(bool last = true);

  public:
    SctpNatPeer();
    virtual ~SctpNatPeer();
    struct pathStatus
    {
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
    /*void setAssociation(SctpAssociation *_assoc) {
       assoc = _assoc;};*/
    void generateAndSend();
    void connect(L3Address connectAddress, int32 connectPort);
    void connectx(AddressVector connectAddressList, int32 connectPort);

    virtual void socketAvailable(SctpSocket *socket, Indication *indication) override { throw cRuntimeError("Model error, this module doesn't use any listener SCTP sockets"); }

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
    void socketFailure(SctpSocket *socket, int32 code) override;

    /** Redefine to handle incoming SctpStatusInfo. */
    void socketStatusArrived(SctpSocket *socket, SctpStatusReq *status) override;
    //@}
    void msgAbandonedArrived(SctpSocket *socket) override;
    //void setAssociation(SctpAssociation *_assoc) {assoc = _assoc;};

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

#endif // ifndef __INET_SCTPNATPEER_H

