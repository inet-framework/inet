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
        struct PathStatus
        {
            Address pid;
            bool active;
            bool primaryPath;
        };
        typedef std::map<Address,PathStatus> SCTPPathStatus;

        // parameters: see the corresponding NED variables
        std::map<unsigned int,unsigned int> streamRequestLengthMap;
        std::map<unsigned int,unsigned int> streamRequestRatioMap;
        std::map<unsigned int,unsigned int> streamRequestRatioSendMap;
        int queueSize;
        unsigned int outStreams;
        unsigned int inStreams;
        bool echo;
        bool ordered;
        bool finishEndsSimulation;

        // state
        SCTPSocket socket;
        SCTPAssociation* assoc;
        SCTPPathStatus sctpPathStatus;
        cMessage* timeMsg;
        cMessage* stopTimer;
        cMessage* primaryChangeTimer;
        int64 bufferSize;
        bool timer;
        bool sendAllowed;

        // statistics
        unsigned long int packetsSent;
        unsigned long int packetsRcvd;
        unsigned long int bytesSent;
        unsigned long int echoedBytesSent;
        unsigned long int bytesRcvd;
        unsigned long int numRequestsToSend; // requests to send in this session
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

        /*
         * Issues an active OPEN to the address/port given as module parameters
         */
        void connect();

        /*
         * Issues CLOSE command
         */
        void close();

        /*
         * Sends a GenericAppMsg of the given length
         */
        //virtual void sendPacket(int numBytes, bool serverClose=false);

        /*
         * When running under GUI, it displays the given string next to the icon
         */
        void setStatusString(const char *s);

        /*
         * Invoked from handleMessage(). Should be redefined to handle self-messages.
         */
        void handleTimer(cMessage *msg);

        /*
         * Does nothing but update statistics/status. Redefine to perform or schedule first sending.
         */
        void socketEstablished(int connId, void *yourPtr, unsigned long int buffer); // TODO: needs a better name

        /*
         * Does nothing but update statistics/status. Redefine to perform or schedule next sending.
         * Beware: this funcion deletes the incoming message, which might not be what you want.
         */
        void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent); // TODO: needs a better name

        void socketDataNotificationArrived(int connId, void *yourPtr, cPacket *msg);

        /*
         * Since remote SCTP closed, invokes close(). Redefine if you want to do something else.
         */
        void socketPeerClosed(int connId, void *yourPtr);

        /*
         * Does nothing but update statistics/status. Redefine if you want to do something else, such as opening a new connection.
         */
        void socketClosed(int connId, void *yourPtr);

        /*
         * Does nothing but update statistics/status. Redefine if you want to try reconnecting after a delay.
         */
        void socketFailure(int connId, void *yourPtr, int code);

        /*
         * Redefine to handle incoming SCTPStatusInfo.
         */
        void socketStatusArrived(int connId, void *yourPtr, SCTPStatusInfo *status);

        // TODO: need to be revised: naming conventions
        void setAssociation(SCTPAssociation *_assoc) {assoc = _assoc;};
        void setPrimaryPath(const char* addr);
        void sendRequestArrived();
        void sendQueueRequest();
        void shutdownReceivedArrived(int connId);
        void sendqueueFullArrived(int connId);
        void sendqueueAbatedArrived(int connId, unsigned long int buffer);
        void addressAddedArrived(int assocId, Address remoteAddr); //XXX this function has no implementation
        void msgAbandonedArrived(int assocId);
        void sendStreamResetNotification();

        /*
         *  Utility: sends a request to the server
         */
        void sendRequest(bool last = true);

        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    public:
        SCTPClient();
        virtual ~SCTPClient();
};

#endif
