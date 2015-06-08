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
#include <omnetpp.h>
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"

namespace inet {

/**
 * Accepts any number of incoming connections, and sends back whatever
 * arrives on them.
 */

class INET_API SCTPNatPeer : public cSimpleModule, public SCTPSocket::CallbackInterface
{
  protected:
    //SCTPAssociation* assoc;
    int32 notifications;
    int32 serverAssocId;
    SCTPSocket clientSocket;
    SCTPSocket peerSocket;
    SCTPSocket rendezvousSocket;
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
    typedef std::map<int32, cDoubleHistogram *> HistEndToEndDelay;
    HistEndToEndDelay histEndToEndDelay;
    typedef std::map<int32, cOutVector *> EndToEndDelay;
    EndToEndDelay endToEndDelay;
    void sendOrSchedule(cMessage *msg);
    void sendRequest(bool last = true);

  public:
    SCTPNatPeer();
    virtual ~SCTPNatPeer();
    struct pathStatus
    {
        bool active;
        bool primaryPath;
        L3Address pid;
    };
    typedef std::map<L3Address, pathStatus> SCTPPathStatus;
    SCTPPathStatus sctpPathStatus;
    //virtual void socketStatusArrived(int32 assocId, void *yourPtr, SCTPStatusInfo *status);
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void handleTimer(cMessage *msg);
    /*void setAssociation(SCTPAssociation *_assoc) {
       assoc = _assoc;};*/
    void generateAndSend();
    void connect(L3Address connectAddress, int32 connectPort);
    void connectx(AddressVector connectAddressList, int32 connectPort);

    /** Does nothing but update statistics/status. Redefine to perform or schedule first sending. */
    void socketEstablished(int32, void *, unsigned long int buffer) override;

    /**
     * Does nothing but update statistics/status. Redefine to perform or schedule next sending.
     * Beware: this funcion deletes the incoming message, which might not be what you want.
     */
    void socketDataArrived(int32 connId, void *yourPtr, cPacket *msg, bool urgent) override;

    void socketDataNotificationArrived(int32 connId, void *yourPtr, cPacket *msg) override;
    /** Since remote SCTP closed, invokes close(). Redefine if you want to do something else. */
    void socketPeerClosed(int32 connId, void *yourPtr) override;

    /** Does nothing but update statistics/status. Redefine if you want to do something else, such as opening a new connection. */
    void socketClosed(int32 connId, void *yourPtr) override;

    /** Does nothing but update statistics/status. Redefine if you want to try reconnecting after a delay. */
    void socketFailure(int32 connId, void *yourPtr, int32 code) override;

    /** Redefine to handle incoming SCTPStatusInfo. */
    void socketStatusArrived(int32 connId, void *yourPtr, SCTPStatusInfo *status) override;
    //@}
    void msgAbandonedArrived(int32 assocId) override;
    //void setAssociation(SCTPAssociation *_assoc) {assoc = _assoc;};

    void setPrimaryPath();
    void sendStreamResetNotification();
    void sendRequestArrived() override;
    void sendQueueRequest();
    void shutdownReceivedArrived(int32 connId) override;
    void sendqueueFullArrived(int32 connId) override;
    void addressAddedArrived(int32 assocId, L3Address localAddr, L3Address remoteAddr) override;
    void setStatusString(const char *s);
};

} // namespace inet

#endif // ifndef __INET_SCTPNATPEER_H

