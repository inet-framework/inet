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

#ifndef __SCTP_H
#define __SCTP_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include <map>

#include "INETDefs.h"

#include "IPvXAddress.h"
#include "UDPSocket.h"

#define SCTP_UDP_PORT  9899

class SCTPAssociation;
class SCTPMessage;


#define sctpEV3 (!SCTP::testing==true)?std::cerr:std::cerr



/**
 * Implements the SCTP protocol. This section describes the internal
 * architecture of the SCTP model.
 *
 *   You may want to check the SCTPSocket
 * class which makes it easier to use SCTP from applications.
 *
 * The SCTP protocol implementation is composed of several classes (discussion
 * follows below):
 *   - SCTP: the module class
 *   - SCTPAssociation: manages a connection
 *   - SCTPSendQueue, SCTPReceiveQueue: abstract base classes for various types
 *      of send and receive queues
 *   - SCTPAlgorithm: abstract base class for SCTP algorithms
 *
 * SCTP subclassed from cSimpleModule. It manages socketpair-to-connection
 * mapping, and dispatches segments and user commands to the appropriate
 * SCTPAssociation object.
 *
 * SCTPAssociation manages the connection, with the help of other objects.
 * SCTPAssociation itself implements the basic SCTP "machinery": takes care
 * of the state machine, stores the state variables (TCB), sends/receives
 *   etc.
 *
 * SCTPAssociation internally relies on 3 objects. The first two are subclassed
 * from SCTPSendQueue and SCTPReceiveQueue. They manage the actual data stream,
 * so SCTPAssociation itself only works with sequence number variables.
 * This makes it possible to easily accomodate need for various types of
 * simulated data transfer: real byte stream, "virtual" bytes (byte counts
 * only), and sequence of cMessage objects (where every message object is
 * mapped to a SCTP sequence number range).
 *
 * Currently implemented send queue and receive queue classes are
 * SCTPVirtualDataSendQueue and SCTPVirtualDataRcvQueue which implement
 * queues with "virtual" bytes (byte counts only).
 *
 * The third object is subclassed from SCTPAlgorithm. Control over
 * retransmissions, congestion control and ACK sending are "outsourced"
 * from SCTPAssociation into SCTPAlgorithm: delayed acks, slow start, fast rexmit,
 * etc. are all implemented in SCTPAlgorithm subclasses.
 *
 * The concrete SCTPAlgorithm class to use can be chosen per connection (in OPEN)
 * or in a module parameter.
 */
class INET_API SCTP : public cSimpleModule
{
    public:
        struct AppConnKey
        {
            int32 appGateIndex;
            int32 assocId;

            inline bool operator<(const AppConnKey& b) const
            {
                if (appGateIndex!=b.appGateIndex)
                    return appGateIndex<b.appGateIndex;
                else
                    return assocId<b.assocId;
            }

        };
        struct SockPair
        {
            IPvXAddress localAddr;
            IPvXAddress remoteAddr;
            uint16 localPort;
            uint16 remotePort;

            inline bool operator<(const SockPair& b) const
            {
                if (remoteAddr!=b.remoteAddr)
                    return remoteAddr<b.remoteAddr;
                else if (localAddr!=b.localAddr)
                    return localAddr<b.localAddr;
                else if (remotePort!=b.remotePort)
                    return remotePort<b.remotePort;
                else
                    return localPort<b.localPort;
            }
        };
        struct VTagPair
        {
            uint32 peerVTag;
            uint32 localVTag;
            uint16 localPort;
            uint16 remotePort;

            /*inline bool operator<(const VTagPair& b) const
            {
                if (peerVTag!=b.peerVTag)
                    return peerVTag<b.peerVTag;
                else if (remotePort!=b.remotePort)
                    return remotePort<b.remotePort;
                else
                    return localPort<b.localPort;
            }*/
        };
        typedef struct
        {
            int32 assocId;
            simtime_t start;
            simtime_t stop;
            uint64 rcvdBytes;
            uint64 sentBytes;
            uint64 transmittedBytes;
            uint64 ackedBytes;
            uint32 numFastRtx;
            uint32 numDups;
            uint32 numT3Rtx;
            uint32 numPathFailures;
            uint32 numForwardTsn;
            double throughput;
            simtime_t lifeTime;
        }AssocStat;

        typedef std::map<int32,AssocStat> AssocStatMap;
        AssocStatMap assocStatMap;
        typedef std::map<int32, VTagPair> SctpVTagMap;
        SctpVTagMap sctpVTagMap;


        typedef std::map<AppConnKey,SCTPAssociation*> SctpAppConnMap;
        typedef std::map<SockPair,SCTPAssociation*> SctpConnMap;

        SctpAppConnMap sctpAppConnMap;
        SctpConnMap sctpConnMap;
        std::list<SCTPAssociation*>assocList;

        UDPSocket udpSocket;

    protected:
        int32 sizeConnMap;
        static int32 nextConnId;

        uint16 nextEphemeralPort;

        SCTPAssociation *findAssocForMessage(IPvXAddress srcAddr, IPvXAddress destAddr, uint32 srcPort, uint32 destPort, bool findListen);
        SCTPAssociation *findAssocForApp(int32 appGateIndex, int32 assocId);
        void sendAbortFromMain(SCTPMessage* sctpmsg, IPvXAddress srcAddr, IPvXAddress destAddr);
        void sendShutdownCompleteFromMain(SCTPMessage* sctpmsg, IPvXAddress srcAddr, IPvXAddress destAddr);
        void updateDisplayString();

    public:
        static bool testing;         // switches between sctpEV and testingEV
        static bool logverbose;  // if !testing, turns on more verbose logging
        void printInfoConnMap();
        void printVTagMap();

        void removeAssociation(SCTPAssociation *assoc);
        simtime_t testTimeout;
        uint32 numGapReports;
        uint32 numPacketsReceived;
        uint32 numPacketsDropped;
        //double failover();
    public:
        //Module_Class_Members(SCTP, cSimpleModule, 0);
        virtual ~SCTP();
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
        virtual void finish();

        inline AssocStat* getAssocStat(uint32 assocId) {
            SCTP::AssocStatMap::iterator found = assocStatMap.find(assocId);
            if (found != assocStatMap.end()) {
              return (&found->second);
            }
            return (NULL);
        }

        /**
        * To be called from SCTPAssociation when socket pair    changes
        */
        void updateSockPair(SCTPAssociation *assoc, IPvXAddress localAddr, IPvXAddress remoteAddr, int32 localPort, int32 remotePort);
        void addLocalAddress(SCTPAssociation *conn, IPvXAddress address);
        void addLocalAddressToAllRemoteAddresses(SCTPAssociation *conn, IPvXAddress address, std::vector<IPvXAddress> remAddresses);
        void addRemoteAddress(SCTPAssociation *conn, IPvXAddress localAddress, IPvXAddress remoteAddress);
        void removeLocalAddressFromAllRemoteAddresses(SCTPAssociation *conn, IPvXAddress address, std::vector<IPvXAddress> remAddresses);
        void removeRemoteAddressFromAllConnections(SCTPAssociation *conn, IPvXAddress address, std::vector<IPvXAddress> locAddresses);
        /**
        * Update assocs socket pair, and register newAssoc (which'll keep LISTENing).
        * Also, assoc will get a new assocId (and newAssoc will live on with its old assocId).
        */
        void addForkedAssociation(SCTPAssociation *assoc, SCTPAssociation *newAssoc, IPvXAddress localAddr, IPvXAddress remoteAddr, int32 localPort, int32 remotePort);

        /**
        * To be called from SCTPAssociation: reserves an ephemeral port for the connection.
        */
        uint16 getEphemeralPort();

        /**
        * Generates a new integer, to be used as assocId. (assocId is part of the key
        * which associates connections with their apps).
        */
        static int32 getNewConnId() {return ++nextConnId;}

        SCTPAssociation* getAssoc(int32 assocId);
        SCTPAssociation *findAssocWithVTag(uint32 peerVTag, uint32 remotePort, uint32 localPort);
        SctpVTagMap getVTagMap() {return sctpVTagMap;};

        void bindPortForUDP();
};

#endif



