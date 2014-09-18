//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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

#ifndef __INET_SCTPCLIENT_H
#define __INET_SCTPCLIENT_H

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

namespace sctp {

class SCTPAssociation;

} // namespace sctp

/**
 * Implements the SCTPClient simple module. See the NED file for more info.
 */
class INET_API SCTPClient : public cSimpleModule, public SCTPSocket::CallbackInterface, public ILifecycle
{
  protected:
    struct PathStatus
    {
        L3Address pid;
        bool active;
        bool primaryPath;
    };
    typedef std::map<L3Address, PathStatus> SCTPPathStatus;

    // parameters: see the corresponding NED variables
    std::map<unsigned int, unsigned int> streamRequestLengthMap;
    std::map<unsigned int, unsigned int> streamRequestRatioMap;
    std::map<unsigned int, unsigned int> streamRequestRatioSendMap;
    int queueSize;
    unsigned int outStreams;
    unsigned int inStreams;
    bool echo;
    bool ordered;
    bool finishEndsSimulation;

    // state
    SCTPSocket socket;
    SCTPPathStatus sctpPathStatus;
    cMessage *timeMsg;
    cMessage *stopTimer;
    cMessage *primaryChangeTimer;
    int64 bufferSize;
    bool timer;
    bool sendAllowed;

    // statistics
    unsigned long int packetsSent;
    unsigned long int packetsRcvd;
    unsigned long int bytesSent;
    unsigned long int echoedBytesSent;
    unsigned long int bytesRcvd;
    unsigned long int numRequestsToSend;    // requests to send in this session
    unsigned long int numPacketsToReceive;
    unsigned int numBytes;
    int numSessions;
    int numBroken;
    int chunksAbandoned;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t echoedPkSignal;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    void initialize(int stage);
    void handleMessage(cMessage *msg);
    void finish();

    void connect();
    void close();
    void setStatusString(const char *s);
    void handleTimer(cMessage *msg);

    /* SCTPSocket::CallbackInterface callback methods */
    void socketEstablished(int connId, void *yourPtr, unsigned long int buffer);    // TODO: needs a better name
    void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);    // TODO: needs a better name
    void socketDataNotificationArrived(int connId, void *yourPtr, cPacket *msg);
    void socketPeerClosed(int connId, void *yourPtr);
    void socketClosed(int connId, void *yourPtr);
    void socketFailure(int connId, void *yourPtr, int code);
    void socketStatusArrived(int connId, void *yourPtr, SCTPStatusInfo *status);

    void setPrimaryPath(const char *addr);
    void sendRequestArrived();
    void sendQueueRequest();
    void shutdownReceivedArrived(int connId);
    void sendqueueAbatedArrived(int connId, unsigned long int buffer);
    void msgAbandonedArrived(int assocId);
    void sendStreamResetNotification();
    void sendRequest(bool last = true);

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    SCTPClient();
    virtual ~SCTPClient();
};

} // namespace inet

#endif // ifndef __INET_SCTPCLIENT_H

