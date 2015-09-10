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

#ifndef __INET_SCTP_H
#define __INET_SCTP_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif // ifdef _MSC_VER

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"

namespace inet {

namespace sctp {

#define SCTP_UDP_PORT    9899

class SCTPAssociation;
class SCTPMessage;

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
 *   - SCTPAssociation: manages an association
 *   - SCTPSendQueue, SCTPReceiveQueue: abstract base classes for various types
 *      of send and receive queues
 *   - SCTPAlgorithm: abstract base class for SCTP algorithms
 *
 * SCTP subclassed from cSimpleModule. It manages socketpair-to-association
 * mapping, and dispatches segments and user commands to the appropriate
 * SCTPAssociation object.
 *
 * SCTPAssociation manages the association, with the help of other objects.
 * SCTPAssociation itself implements the basic SCTP "machinery": takes care
 * of the state machine, stores the state variables (TCB), sends/receives
 * etc.
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
 * The concrete SCTPAlgorithm class to use can be chosen per association (in OPEN)
 * or in a module parameter.
 */
class INET_API SCTP : public cSimpleModule
{
  public:
    struct AppAssocKey
    {
        int32 appGateIndex;
        int32 assocId;

        inline bool operator<(const AppAssocKey& b) const
        {
            if (appGateIndex != b.appGateIndex)
                return appGateIndex < b.appGateIndex;
            else
                return assocId < b.assocId;
        }
    };
    struct SockPair
    {
        L3Address localAddr;
        L3Address remoteAddr;
        uint16 localPort;
        uint16 remotePort;

        inline bool operator<(const SockPair& b) const
        {
            if (remoteAddr != b.remoteAddr)
                return remoteAddr < b.remoteAddr;
            else if (localAddr != b.localAddr)
                return localAddr < b.localAddr;
            else if (remotePort != b.remotePort)
                return remotePort < b.remotePort;
            else
                return localPort < b.localPort;
        }
    };
    struct VTagPair
    {
        uint32 peerVTag;
        uint32 localVTag;
        uint16 localPort;
        uint16 remotePort;
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
        uint32 numOverfullSACKs;
        uint64 sumRGapRanges;    // Total sum of RGap ranges (Last RGapStop - CumAck)
        uint64 sumNRGapRanges;    // Total sum of NRGap ranges (Last NRGapStop - CumAck)
        uint32 numDropsBecauseNewTSNGreaterThanHighestTSN;
        uint32 numDropsBecauseNoRoomInBuffer;
        uint32 numChunksReneged;
        uint32 numAuthChunksSent;
        uint32 numAuthChunksAccepted;
        uint32 numAuthChunksRejected;
        uint32 numResetRequestsSent;
        uint32 numResetRequestsPerformed;
        simtime_t fairStart;
        simtime_t fairStop;
        uint64 fairAckedBytes;
        double fairThroughput;
        simtime_t fairLifeTime;
        uint64 numEndToEndMessages;
        SimTime cumEndToEndDelay;
        uint64 startEndToEndDelay;
        uint64 stopEndToEndDelay;
    } AssocStat;

    typedef std::map<int32, AssocStat> AssocStatMap;
    AssocStatMap assocStatMap;
    typedef std::map<int32, VTagPair> SctpVTagMap;
    SctpVTagMap sctpVTagMap;

    typedef std::map<AppAssocKey, SCTPAssociation *> SctpAppAssocMap;
    typedef std::map<SockPair, SCTPAssociation *> SctpAssocMap;

    SctpAppAssocMap sctpAppAssocMap;
    SctpAssocMap sctpAssocMap;
    std::list<SCTPAssociation *> assocList;

    UDPSocket udpSocket;

  protected:
    IRoutingTable *rt;
    IInterfaceTable *ift;

    int32 sizeAssocMap;

    uint16 nextEphemeralPort;

    SCTPAssociation *findAssocForMessage(L3Address srcAddr, L3Address destAddr, uint32 srcPort, uint32 destPort, bool findListen);
    SCTPAssociation *findAssocForApp(int32 appGateIndex, int32 assocId);
    void sendAbortFromMain(SCTPMessage *sctpmsg, L3Address fromAddr, L3Address toAddr);
    void sendShutdownCompleteFromMain(SCTPMessage *sctpmsg, L3Address fromAddr, L3Address toAddr);
    void updateDisplayString();

  public:
    void printInfoAssocMap();
    void printVTagMap();

    void removeAssociation(SCTPAssociation *assoc);
    simtime_t testTimeout;
    uint32 numGapReports;
    uint32 numPacketsReceived;
    uint32 numPacketsDropped;
    bool auth;
    bool addIP;
    bool pktdrop;
    bool sackNow;
    uint64 numPktDropReports;

  public:
    virtual ~SCTP();
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void send_to_ip(SCTPMessage *msg);


    inline AssocStat *getAssocStat(uint32 assocId)
    {
        auto found = assocStatMap.find(assocId);
        if (found != assocStatMap.end()) {
            return &found->second;
        }
        return nullptr;
    }

    /**
     * To be called from SCTPAssociation when socket pair    changes
     */
    void updateSockPair(SCTPAssociation *assoc, L3Address localAddr, L3Address remoteAddr, int32 localPort, int32 remotePort);
    void addLocalAddress(SCTPAssociation *assoc, L3Address address);
    void addLocalAddressToAllRemoteAddresses(SCTPAssociation *assoc, L3Address address, std::vector<L3Address> remAddresses);
    bool addRemoteAddress(SCTPAssociation *assoc, L3Address localAddress, L3Address remoteAddress);
    void removeLocalAddressFromAllRemoteAddresses(SCTPAssociation *assoc, L3Address address, std::vector<L3Address> remAddresses);
    void removeRemoteAddressFromAllAssociations(SCTPAssociation *assoc, L3Address address, std::vector<L3Address> locAddresses);
    /**
     * Update assocs socket pair, and register newAssoc (which'll keep LISTENing).
     * Also, assoc will get a new assocId (and newAssoc will live on with its old assocId).
     */
    void addForkedAssociation(SCTPAssociation *assoc, SCTPAssociation *newAssoc, L3Address localAddr, L3Address remoteAddr, int32 localPort, int32 remotePort);

    /**
     * To be called from SCTPAssociation: reserves an ephemeral port for the connection.
     */
    uint16 getEphemeralPort();

    SCTPAssociation *getAssoc(int32 assocId);
    SCTPAssociation *findAssocWithVTag(uint32 peerVTag, uint32 remotePort, uint32 localPort);

    SCTPAssociation *findAssocForInitAck(SCTPInitAckChunk *initack, L3Address srcAddr, L3Address destAddr, uint32 srcPort, uint32 destPort, bool findListen);

    SctpVTagMap getVTagMap() { return sctpVTagMap; };

    void bindPortForUDP();
};

} // namespace sctp

} // namespace inet

#endif // ifndef __INET_SCTP_H

