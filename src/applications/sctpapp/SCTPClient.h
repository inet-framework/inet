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

#ifndef __SCTPCLIENT_H_
#define __SCTPCLIENT_H_

#include "INETDefs.h"
#include "SCTPSocket.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"

class SCTPAssociation;

/**
 * Implements the SCTPClient simple module. See the NED file for more info.
 */
class INET_API SCTPClient : public cSimpleModule, public SCTPSocket::CallbackInterface, public ILifecycle
{
    protected:
        SCTPSocket socket;
        SCTPAssociation* assoc;
        // statistics
        int32 numSessions;
        int32 numBroken;
        uint64 packetsSent;
        uint64 packetsRcvd;
        uint64 bytesSent;
        uint64 echoedBytesSent;
        uint64 bytesRcvd;
        uint64 numRequestsToSend; // requests to send in this session
        uint64 numPacketsToReceive;
        uint32 numBytes;
        int64 bufferSize;
        int32 queueSize;
        uint32 inStreams;
        uint32 outStreams;

        static simsignal_t sentPkSignal;
        static simsignal_t rcvdPkSignal;
        static simsignal_t echoedPkSignal;

        bool ordered;
        bool sendAllowed;
        bool timer;
        bool finishEndsSimulation;
        bool echo;
        cMessage* timeMsg;
        cMessage* stopTimer;
        cMessage* primaryChangeTimer;

        int32 chunksAbandoned;
        std::map<uint32,uint32> streamRequestLengthMap;
        std::map<uint32,uint32> streamRequestRatioMap;
        std::map<uint32,uint32> streamRequestRatioSendMap;

        /** Utility: sends a request to the server */
        void sendRequest(bool last = true);

    public:
        SCTPClient();
        virtual ~SCTPClient();
        struct pathStatus
        {
            bool active;
            bool primaryPath;
            IPvXAddress  pid;
        };

        typedef std::map<IPvXAddress,pathStatus> SCTPPathStatus;
        SCTPPathStatus sctpPathStatus;

        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    protected:
        virtual int numInitStages() const { return 4; }

        /**
         * Initialization.
         */
        void initialize(int stage);

        /**
         * For self-messages it invokes handleTimer(); messages arriving from SCTP
         * will get dispatched to the socketXXX() functions.
         */
        void handleMessage(cMessage *msg);

        /**
         * Records basic statistics: numSessions, packetsSent, packetsRcvd,
         * bytesSent, bytesRcvd. Redefine to record different or more statistics
         * at the end of the simulation.
         */
        void finish();

        /** @name Utility functions */
        //@{
        /** Issues an active OPEN to the address/port given as module parameters */
        void connect();

        /** Issues CLOSE command */
        void close();

        /** Sends a GenericAppMsg of the given length */
        //virtual void sendPacket(int32 numBytes, bool serverClose=false);

        /** When running under GUI, it displays the given string next to the icon */
        void setStatusString(const char *s);
        //@}

        /** Invoked from handleMessage(). Should be redefined to handle self-messages. */
        void handleTimer(cMessage *msg);

        /** @name SCTPSocket::CallbackInterface callback methods */
        //@{

        /** Does nothing but update statistics/status. Redefine to perform or schedule first sending. */
        void socketEstablished(int32 connId, void *yourPtr, uint64 buffer);

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

        void setAssociation(SCTPAssociation *_assoc) {assoc = _assoc;};
        void setPrimaryPath(const char* addr);
        void sendRequestArrived();
        void sendQueueRequest();
        void shutdownReceivedArrived(int32 connId);
        void sendqueueFullArrived(int32 connId);
        void sendqueueAbatedArrived(int32 connId, uint64 buffer);
        void addressAddedArrived(int32 assocId, IPvXAddress remoteAddr);
        void msgAbandonedArrived(int32 assocId);
        void sendStreamResetNotification();
};

#endif

