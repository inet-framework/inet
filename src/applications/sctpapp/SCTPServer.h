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

#ifndef __SCTPSERVER_H_
#define __SCTPSERVER_H_

#include "INETDefs.h"
#include "SCTPAssociation.h"
#include "SCTPSocket.h"
#include "ILifecycle.h"
#include "LifecycleOperation.h"

/**
 * Implements the SCTPServer simple module. See the NED file for more info.
 */
class INET_API SCTPServer : public cSimpleModule, public ILifecycle
{
    protected:
        struct ServerAssocStat
        {
            simtime_t start;
            simtime_t stop;
            simtime_t lifeTime;
            uint64 rcvdBytes;
            uint64 sentPackets;
            uint64 rcvdPackets;
            bool abortSent;
            bool peerClosed;
        };
        typedef std::map<int32,ServerAssocStat> ServerAssocStatMap;
        typedef std::map<int32,cOutVector*> BytesPerAssoc;
        typedef std::map<int32,cOutVector*> EndToEndDelay;

        // parameters
        int32 inboundStreams;
        int32 outboundStreams;
        int32 queueSize;
        double delay;
        double delayFirstRead;
        bool finishEndsSimulation;
        bool echo;
        bool ordered;

        // state
        SCTPSocket *socket;
        cMessage *timeoutMsg;
        cMessage *delayTimer;
        cMessage *delayFirstReadTimer;
        int32 lastStream;
        int32 assocId;
        bool readInt;
        bool schedule;
        bool firstData;
        bool shutdownReceived;
        bool abortSent;
        EndToEndDelay endToEndDelay;

        // statistics
        int32 numSessions;
        int32 count;
        int32 notificationsReceived;
        uint64 bytesSent;
        uint64 packetsSent;
        uint64 packetsRcvd;
        uint64 numRequestsToSend; // requests to send in this session
        BytesPerAssoc bytesPerAssoc;
        ServerAssocStatMap serverAssocStatMap;

    protected:
        void sendOrSchedule(cPacket *msg);
        cPacket* makeAbortNotification(SCTPCommand* msg);
        cPacket* makeReceiveRequest(cPacket* msg);
        cPacket* makeDefaultReceive();
        int32 ssn;

        virtual void initialize(int stage);
        virtual int numInitStages() const { return NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage *msg);
        virtual void finish();
        void handleTimer(cMessage *msg);
        void generateAndSend();
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
        { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

    public:
       virtual ~SCTPServer();
};
#endif


