//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCTP_H
#define __INET_SCTP_H

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif // ifdef _MSC_VER

#include <map>

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/sctp/SctpCrcInsertionHook.h"
#include "inet/transportlayer/sctp/SctpHeader.h"
#include "inet/transportlayer/sctp/SctpUdpHook.h"

namespace inet {
namespace sctp {

#define SCTP_UDP_PORT    9899

class SctpAssociation;
class SctpHeader;

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
 *   - SctpAssociation: manages an association
 *   - SctpSendQueue, SctpReceiveQueue: abstract base classes for various types
 *      of send and receive queues
 *   - SctpAlgorithm: abstract base class for SCTP algorithms
 *
 * SCTP subclassed from cSimpleModule. It manages socketpair-to-association
 * mapping, and dispatches segments and user commands to the appropriate
 * SctpAssociation object.
 *
 * SctpAssociation manages the association, with the help of other objects.
 * SctpAssociation itself implements the basic SCTP "machinery": takes care
 * of the state machine, stores the state variables (TCB), sends/receives
 * etc.
 *
 * SctpAssociation internally relies on 3 objects. The first two are subclassed
 * from SctpSendQueue and SctpReceiveQueue. They manage the actual data stream,
 * so SctpAssociation itself only works with sequence number variables.
 * This makes it possible to easily accomodate need for various types of
 * simulated data transfer: real byte stream, "virtual" bytes (byte counts
 * only), and sequence of cMessage objects (where every message object is
 * mapped to a SCTP sequence number range).
 *
 * Currently implemented send queue and receive queue classes are
 * SctpVirtualDataSendQueue and SctpVirtualDataRcvQueue which implement
 * queues with "virtual" bytes (byte counts only).
 *
 * The third object is subclassed from SctpAlgorithm. Control over
 * retransmissions, congestion control and ACK sending are "outsourced"
 * from SctpAssociation into SctpAlgorithm: delayed acks, slow start, fast rexmit,
 * etc. are all implemented in SctpAlgorithm subclasses.
 *
 * The concrete SctpAlgorithm class to use can be chosen per association (in OPEN)
 * or in a module parameter.
 */
class INET_API Sctp : public cSimpleModule
{
  public:
    struct AppAssocKey {
        int32_t appGateIndex;
        int32_t assocId;

        bool operator<(const AppAssocKey& b) const
        {
            if (appGateIndex != b.appGateIndex)
                return appGateIndex < b.appGateIndex;
            else
                return assocId < b.assocId;
        }
    };

    struct SockPair {
        L3Address localAddr;
        L3Address remoteAddr;
        uint16_t localPort;
        uint16_t remotePort;

        bool operator<(const SockPair& b) const
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

    struct VTagPair {
        uint32_t peerVTag;
        uint32_t localVTag;
        uint16_t localPort;
        uint16_t remotePort;
    };

    struct AssocStat {
        int32_t assocId;
        simtime_t start;
        simtime_t stop;
        uint64_t rcvdBytes;
        uint64_t sentBytes;
        uint64_t transmittedBytes;
        uint64_t ackedBytes;
        uint32_t numFastRtx;
        uint32_t numDups;
        uint32_t numT3Rtx;
        uint32_t numPathFailures;
        uint32_t numForwardTsn;
        double throughput;
        simtime_t lifeTime;
        uint32_t numOverfullSACKs;
        uint64_t sumRGapRanges; // Total sum of RGap ranges (Last RGapStop - CumAck)
        uint64_t sumNRGapRanges; // Total sum of NRGap ranges (Last NRGapStop - CumAck)
        uint32_t numDropsBecauseNewTsnGreaterThanHighestTsn;
        uint32_t numDropsBecauseNoRoomInBuffer;
        uint32_t numChunksReneged;
        uint32_t numAuthChunksSent;
        uint32_t numAuthChunksAccepted;
        uint32_t numAuthChunksRejected;
        uint32_t numResetRequestsSent;
        uint32_t numResetRequestsPerformed;
        simtime_t fairStart;
        simtime_t fairStop;
        uint64_t fairAckedBytes;
        double fairThroughput;
        simtime_t fairLifeTime;
        uint64_t numEndToEndMessages;
        SimTime cumEndToEndDelay;
        uint64_t startEndToEndDelay;
        uint64_t stopEndToEndDelay;
    };

    typedef std::map<int32_t, AssocStat> AssocStatMap;
    AssocStatMap assocStatMap;
    typedef std::map<int32_t, VTagPair> SctpVTagMap;
    SctpVTagMap sctpVTagMap;

    typedef std::map<AppAssocKey, SctpAssociation *> SctpAppAssocMap;
    typedef std::map<SockPair, SctpAssociation *> SctpAssocMap;

    SctpAppAssocMap sctpAppAssocMap;
    SctpAssocMap sctpAssocMap;
    std::list<SctpAssociation *> assocList;

    UdpSocket udpSocket;
    int udpSockId;

    SocketOptions *socketOptions = nullptr;

  protected:
    ModuleRefByPar<IRoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;

    int32_t sizeAssocMap;

    uint16_t nextEphemeralPort;

    SctpAssociation *findAssocForMessage(L3Address srcAddr, L3Address destAddr, uint32_t srcPort, uint32_t destPort, bool findListen);
    SctpAssociation *findAssocForApp(int32_t appGateIndex, int32_t assocId);
    int32_t findAssocForFd(int32_t fd);
    void sendAbortFromMain(SctpHeader *sctpMsg, L3Address fromAddr, L3Address toAddr);
    void sendShutdownCompleteFromMain(SctpHeader *sctpMsg, L3Address fromAddr, L3Address toAddr);
    virtual void refreshDisplay() const override;

  public:
    void printInfoAssocMap();
    void printVTagMap();

    void removeAssociation(SctpAssociation *assoc);
    simtime_t testTimeout;
    uint32_t numGapReports;
    uint32_t numPacketsReceived;
    uint32_t numPacketsDropped;
    bool auth;
    bool addIP;
    bool pktdrop;
    bool sackNow;
    uint64_t numPktDropReports;
    int interfaceId = -1;
    CrcMode crcMode = CRC_MODE_UNDEFINED;

  public:
    virtual ~Sctp();
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void send_to_ip(Packet *msg);

    AssocStat *getAssocStat(uint32_t assocId)
    {
        auto found = assocStatMap.find(assocId);
        return (found != assocStatMap.end()) ? &found->second : nullptr;
    }

    /**
     * To be called from SctpAssociation when socket pair    changes
     */
    void updateSockPair(SctpAssociation *assoc, L3Address localAddr, L3Address remoteAddr, int32_t localPort, int32_t remotePort);
    void addLocalAddress(SctpAssociation *assoc, L3Address address);
    void addLocalAddressToAllRemoteAddresses(SctpAssociation *assoc, L3Address address, std::vector<L3Address> remAddresses);
    bool addRemoteAddress(SctpAssociation *assoc, L3Address localAddress, L3Address remoteAddress);
    void removeLocalAddressFromAllRemoteAddresses(SctpAssociation *assoc, L3Address address, std::vector<L3Address> remAddresses);
    void removeRemoteAddressFromAllAssociations(SctpAssociation *assoc, L3Address address, std::vector<L3Address> locAddresses);
    /**
     * Update assocs socket pair, and register newAssoc (which'll keep LISTENing).
     * Also, assoc will get a new assocId (and newAssoc will live on with its old assocId).
     */
    void addForkedAssociation(SctpAssociation *assoc, SctpAssociation *newAssoc, L3Address localAddr, L3Address remoteAddr, int32_t localPort, int32_t remotePort);

    /**
     * To be called from SctpAssociation: reserves an ephemeral port for the connection.
     */
    uint16_t getEphemeralPort();

    SctpAssociation *getAssoc(int32_t assocId);
    SctpAssociation *findAssocWithVTag(uint32_t peerVTag, uint32_t remotePort, uint32_t localPort);

    SctpAssociation *findAssocForInitAck(SctpInitAckChunk *initack, L3Address srcAddr, L3Address destAddr, uint32_t srcPort, uint32_t destPort, bool findListen);

    SctpVTagMap getVTagMap() { return sctpVTagMap; };

    void bindPortForUDP();

    /** Getter and Setter for the socket options **/
    SocketOptions *collectSocketOptions();

    void setSocketOptions(SocketOptions *options) { delete socketOptions; socketOptions = options; }
    int getMaxInitRetrans() { return socketOptions->maxInitRetrans; };
    int getMaxInitRetransTimeout() { return socketOptions->maxInitRetransTimeout; };
    double getRtoInitial() { return socketOptions->rtoInitial; };
    double getRtoMin() { return socketOptions->rtoMin; };
    double getRtoMax() { return socketOptions->rtoMax; };
    int getSackFrequency() { return socketOptions->sackFrequency; };
    double getSackPeriod() { return socketOptions->sackPeriod; };
    int getMaxBurst() { return socketOptions->maxBurst; };
    int getFragPoint() { return socketOptions->fragPoint; };
    int getNagle() { return socketOptions->nagle; };
    bool getEnableHeartbeats() { return socketOptions->enableHeartbeats; };
    int getPathMaxRetrans() { return socketOptions->pathMaxRetrans; };
    int getAssocMaxRtx() { return socketOptions->assocMaxRtx; };
    double getHbInterval() { return socketOptions->hbInterval; };
    void setRtoInitial(double rtoInitial) { socketOptions->rtoInitial = rtoInitial; }
    void setRtoMin(double rtoMin) { socketOptions->rtoMin = rtoMin; }
    void setRtoMax(double rtoMax) { socketOptions->rtoMax = rtoMax; }
    void setInterfaceId(int id) { interfaceId = id; }
    int getInterfaceId() { return interfaceId; };
};

} // namespace sctp
} // namespace inet

#endif

