//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_SCTPPEER_H
#define __INET_SCTPPEER_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

class SCTPConnectInfo;

/**
 * Implements the SCTPPeer simple module. See the NED file for more info.
 */
class INET_API SCTPPeer : public cSimpleModule, public SCTPSocket::CallbackInterface, public ILifecycle
{
  protected:
    struct PathStatus
    {
        bool active;
        bool primaryPath;
        IPv4Address pid;
    };
    typedef std::map<int, long> RcvdPacketsPerAssoc;
    typedef std::map<int, long> SentPacketsPerAssoc;
    typedef std::map<int, long> RcvdBytesPerAssoc;
    typedef std::map<int, cOutVector *> BytesPerAssoc;
    typedef std::map<int, cDoubleHistogram *> HistEndToEndDelay;
    typedef std::map<int, cOutVector *> EndToEndDelay;
    typedef std::map<L3Address, PathStatus> SCTPPathStatus;

    // parameters
    double delay;
    bool echo;
    bool ordered;
    bool schedule;
    int queueSize;
    int outboundStreams;

    // state
    SCTPPathStatus sctpPathStatus;
    SCTPSocket clientSocket;
    cMessage *timeoutMsg;
    cMessage *timeMsg;
    cMessage *connectTimer;
    bool shutdownReceived;
    bool sendAllowed;
    int serverAssocId;
    int clientAssocId;
    int numRequestsToSend;    // requests to send in this session
    int lastStream;
    int numPacketsToReceive;
    int ssn;

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
    static simsignal_t sentPkSignal;
    static simsignal_t echoedPkSignal;
    static simsignal_t rcvdPkSignal;

  protected:

    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    void handleTimer(cMessage *msg);

    void connect();
    void socketEstablished(int connId, void *yourPtr);
    void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    void socketDataNotificationArrived(int connId, void *yourPtr, cPacket *msg);
    void socketPeerClosed(int connId, void *yourPtr);
    void socketClosed(int connId, void *yourPtr);
    void socketFailure(int connId, void *yourPtr, int code);

    /* Redefine to handle incoming SCTPStatusInfo */
    void socketStatusArrived(int connId, void *yourPtr, SCTPStatusInfo *status);

    void sendRequest(bool last = true);
    void sendOrSchedule(cPacket *msg);
    void generateAndSend(SCTPConnectInfo *connectInfo);
    void sendRequestArrived();
    void sendQueueRequest();
    void shutdownReceivedArrived(int connId);
    void sendqueueFullArrived(int connId);
    void msgAbandonedArrived(int assocId);
    void setStatusString(const char *s);

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    SCTPPeer();
    ~SCTPPeer();
};

} // namespace inet

#endif // ifndef __INET_SCTPPEER_H

