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

#ifndef __SCTPPEER_H_
#define __SCTPPEER_H_

#include "INETDefs.h"
#include "SCTPAssociation.h"
#include "SCTPSocket.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"

class SCTPConnectInfo;

/**
 * Implements the SCTPPeer simple module. See the NED file for more info.
 */
class INET_API SCTPPeer : public cSimpleModule, public SCTPSocket::CallbackInterface, public ILifecycle
{
    protected:
        int32 notifications;
        int32 serverAssocId;
        int32 clientAssocId;
        SCTPSocket clientSocket;
        double delay;
        bool echo;
        bool schedule;
        bool shutdownReceived;
        bool ordered;
        bool sendAllowed;
        long bytesSent;
        int32 packetsSent;
        int32 packetsRcvd;
        int32 numSessions;
        int32 numRequestsToSend; // requests to send in this session
        int32 queueSize;
        cMessage *timeoutMsg;
        int32 outboundStreams;
        cMessage *timeMsg;
        cMessage *connectTimer;
        int32 bytesRcvd;
        int32 echoedBytesSent;
        int32 lastStream;
        int32 numPacketsToReceive;
        int32 chunksAbandoned;

        // statistics
        typedef std::map<int32,long> RcvdPacketsPerAssoc;
        RcvdPacketsPerAssoc rcvdPacketsPerAssoc;

        typedef std::map<int32,long> SentPacketsPerAssoc;
        SentPacketsPerAssoc sentPacketsPerAssoc;

        typedef std::map<int32,long> RcvdBytesPerAssoc;
        RcvdBytesPerAssoc rcvdBytesPerAssoc;

        typedef std::map<int32,cOutVector*> BytesPerAssoc;
        BytesPerAssoc bytesPerAssoc;

        typedef std::map<int32,cDoubleHistogram*> HistEndToEndDelay;
        HistEndToEndDelay histEndToEndDelay;

        typedef std::map<int32,cOutVector*> EndToEndDelay;
        EndToEndDelay endToEndDelay;

    protected:
        void sendOrSchedule(cPacket *msg);
        void sendRequest(bool last = true);
        int32 ssn;
        static simsignal_t sentPkSignal;
        static simsignal_t echoedPkSignal;
        static simsignal_t rcvdPkSignal;

    public:
        struct pathStatus {
            bool active;
            bool primaryPath;
            IPv4Address  pid;
        };
        typedef std::map<IPvXAddress,pathStatus> SCTPPathStatus;
        SCTPPathStatus sctpPathStatus;

    public:
        SCTPPeer();
        ~SCTPPeer();

        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    protected:
        virtual void initialize(int stage);
        virtual int numInitStages() const { return 4; }
        virtual void handleMessage(cMessage *msg);
        virtual void finish();
        void handleTimer(cMessage *msg);
        void generateAndSend(SCTPConnectInfo *connectInfo);
        void connect();

        /** Does nothing but update statistics/status. Redefine to perform or schedule first sending. */
        void socketEstablished(int32 connId, void *yourPtr);

        /**
        * Does nothing but update statistics/status. Redefine to perform or schedule next sending.
        * Beware: this funcion deletes the incoming message, which might not be what you want.
        */
        void socketDataArrived(int32 connId, void *yourPtr, cPacket *msg, bool urgent);

        void socketDataNotificationArrived(int32 connId, void *yourPtr, cPacket *msg);
        /** Since remote SCTP closed, invokes close(). Redefine if you want to do something else. */
        void socketPeerClosed(int32 connId, void *yourPtr);

        /** Does nothing but update statistics/status. Redefine if you want to do something else, such as opening a new connection. */
        void socketClosed(int32 connId, void *yourPtr);

        /** Does nothing but update statistics/status. Redefine if you want to try reconnecting after a delay. */
        void socketFailure(int32 connId, void *yourPtr, int32 code);

        /** Redefine to handle incoming SCTPStatusInfo. */
        void socketStatusArrived(int32 connId, void *yourPtr, SCTPStatusInfo *status);
        //@}
        void setPrimaryPath();
        void sendRequestArrived();
        void sendQueueRequest();
        void shutdownReceivedArrived(int32 connId);
        void sendqueueFullArrived(int32 connId);
        void msgAbandonedArrived(int32 assocId);
        void sendStreamResetNotification();

        void setStatusString(const char *s);
        void addressAddedArrived(int32 assocId, IPvXAddress remoteAddr);
};

#endif


