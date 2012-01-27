//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
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


/**
 * Implements the SCTPServer simple module. See the NED file for more info.
 */
class INET_API SCTPServer : public cSimpleModule
{
    protected:
        int32 notifications;
        int32 assocId;
        SCTPSocket *socket;
        double delay;
        double delayFirstRead;
        bool readInt;
        bool schedule;
        bool firstData;
        bool shutdownReceived;
        bool echo;
        bool finishEndsSimulation;
        bool ordered;
        bool abortSent;
        uint64 bytesSent;
        uint64 packetsSent;
        uint64 packetsRcvd;
        uint64 numRequestsToSend; // requests to send in this session
        int32 numSessions;
        int32 queueSize;
        int32 count;
        cMessage *timeoutMsg;
        cMessage *delayTimer;
        cMessage *delayFirstReadTimer;
        //cPacket* abort;
        int32 inboundStreams;
        int32 outboundStreams;
        int32 lastStream;

        struct ServerAssocStat
        {
            simtime_t start;
            simtime_t stop;
            uint64 rcvdBytes;
            uint64 sentPackets;
            uint64 rcvdPackets;
            simtime_t lifeTime;
            bool abortSent;
            bool peerClosed;
        };

        typedef std::map<int32,ServerAssocStat> ServerAssocStatMap;
        ServerAssocStatMap serverAssocStatMap;

        typedef std::map<int32,cOutVector*> BytesPerAssoc;
        BytesPerAssoc bytesPerAssoc;

        typedef std::map<int32,cDoubleHistogram*> HistEndToEndDelay;
        HistEndToEndDelay histEndToEndDelay;

        typedef std::map<int32,cOutVector*> EndToEndDelay;
        EndToEndDelay endToEndDelay;

    protected:
        void sendOrSchedule(cPacket *msg);
        cPacket* makeAbortNotification(SCTPCommand* msg);
        cPacket* makeReceiveRequest(cPacket* msg);
        cPacket* makeDefaultReceive();
        int32 ssn;

    public:
        virtual ~SCTPServer();
        void initialize();
        void handleMessage(cMessage *msg);
        void finish();
        void handleTimer(cMessage *msg);
        void generateAndSend();
};

#endif


