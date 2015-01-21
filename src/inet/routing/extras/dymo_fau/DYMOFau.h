/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __INET_DYMO_H
#define __INET_DYMO_H

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/ipv4/ICMPMessage.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

#include "inet/routing/extras/base/ManetRoutingBase.h"

#include "inet/routing/extras/dymo_fau/DYMO_Packet_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_RoutingTable.h"
#include "inet/routing/extras/dymo_fau/DYMO_OutstandingRREQList.h"
#include "inet/routing/extras/dymo_fau/DYMO_RM_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_RREQ_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_RREP_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_AddressBlock.h"
#include "inet/routing/extras/dymo_fau/DYMO_RERR_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_UERR_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_DataQueue.h"
#include "inet/routing/extras/dymo_fau/DYMO_Timeout_m.h"
#include "inet/routing/extras/dymo_fau/DYMO_TokenBucket.h"

namespace inet {

namespace inetmanet {

//===========================================================================================
// class QueueElement: defines a data type for all queued data messages
//===========================================================================================
class QueueElement : public cPacket
{
  public:
    QueueElement() { seqNum = 0; destAddr = 0; srcAddr = 0; obj = nullptr; };

    QueueElement(unsigned int seqNum, unsigned int destAddr, unsigned int srcAddr, DYMO_Packet* obj)
    {
        this->seqNum = seqNum;
        this->destAddr = destAddr;
        this->srcAddr = srcAddr;
        this->obj = obj;
    };
    virtual ~QueueElement() {};
    unsigned int getSeqNum() { return seqNum; };
    unsigned int getDestAddr() { return destAddr; };
    unsigned int getSrcAddr() { return srcAddr; };
    DYMO_Packet* getObject() { return obj; };

  private:
    unsigned int seqNum;
    unsigned int destAddr;
    unsigned int srcAddr;
    DYMO_Packet* obj;
};

//===========================================================================================
// class DYMOFau: implements the network layer to route incoming messages
//===========================================================================================
class DYMOFau : public ManetRoutingBase
{
  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;
    virtual void finish() override;

    DYMOFau();
    ~DYMOFau();

    /** @brief Function called whenever a message arrives at the module */
    void handleMessage(cMessage * msg) override;

    void onOutboundDataPacket(const cPacket* packet);


    /** @brief change the address we think we are reachable at. Does not(!) make sure we are actually reachable there. The change request will be ignored if this node has already participated in DYMO */
    void setMyAddr(unsigned int myAddr);

    /** @brief ... */
    DYMO_RoutingTable* getDYMORoutingTable();

    /** @brief guesses which router the given address belongs to, might return 0 */
    cModule* getRouterByAddress(IPv4Address address);


  private:
    friend class DYMO_RoutingTable;

    void processPacket(const IPv4Datagram* datagram);
    //===============================================================================
    // OPERATIONS
    //===============================================================================


    /** @brief Handle self messages such as timer... */
    virtual void handleSelfMsg(cMessage*);

    /** @brief return destination interface for reply messages */
    InterfaceEntry* getNextHopInterface(DYMO_PacketBBMessage* pkt);

    /** @brief return destination address for reply messages */
    uint32_t getNextHopAddress(DYMO_RM *routingMsg);

    /** @brief Function handles lower messages: sends messages sent to the node to higher layer, forward messages to destination nodes... */
    virtual void handleLowerMsg(cPacket*);
    virtual void handleLowerRM(DYMO_RM*);
    virtual void handleLowerRMForMe(DYMO_RM *routingMsg);
    virtual void handleLowerRMForRelay(DYMO_RM *routingMsg);
    virtual void handleLowerRERR(DYMO_RERR*);
    virtual void handleLowerUERR(DYMO_UERR*);

    /** @brief Function sends messages to lower layer (transport layer) */
    void sendDown(cPacket*, int);

    /** @brief Increments the ownSeqNum */
    void incSeqNum();

    /** @brief Returns true if seqNumInQuestion is greater than referenceSeqNum, taking into account sequence number rollover */
    bool seqNumIsFresher(unsigned int seqNumInQuestion, unsigned int referenceSeqNum);

    /** @brief calculates a backoff value */
    simtime_t computeBackoff(simtime_t);

    /** @brief updates the lifetime (validTimeout) of a route */
    void updateRouteLifetimes(const L3Address& destAddr);

    /** @brief generates and sends down a new rreq for given destination address */
    void sendRREQ(unsigned int destAddr, int msgHdrHopLimit, unsigned int targetSeqNum, unsigned int targetHopCnt);

    /** @brief Creates and sends a RREP to the given destination */
    void sendReply(unsigned int destAddr, unsigned int tSeqNum);

    /** @brief Creates and sends a RREP when this node is an intermediate router */
    void sendReplyAsIntermediateRouter(const DYMO_AddressBlock& origNode, const DYMO_AddressBlock& targetNode, const DYMO_RoutingEntry* routeToTarget);

    /** @brief generates and sends a RERR message */
    void sendRERR(unsigned int targetAddress, unsigned int targetSeqNum);

    /** @brief checks whether an AddressBlock's information is better than the current one or is stale/disregarded. Behaviour according to draft-ietf-manet-dymo-09, section 5.2.1 ("Judging New Routing Information's Usefulness") */
    bool isRBlockBetter(DYMO_RoutingEntry * entry, DYMO_AddressBlock rblock, bool isRREQ);

    /** @brief Function verifies the send tries for a given RREQ and send a new request if RREQ_TRIES is not reached, deletes conrol info from the queue if RREQ_TRIES is reached  and send an ICMP message to upper layer */
    void handleRREQTimeout(DYMO_OutstandingRREQ& outstandingRREQ);

    /** @brief updates routing entries from AddressBlock, returns whether AddressBlock should be kept */
    bool updateRoutesFromAddressBlock(const DYMO_AddressBlock& ab, bool isRREQ, uint32_t nextHopAddress, InterfaceEntry* nextHopInterface);

    /** @brief Function updates the routing entries from received AdditionalNodes. @see draft 4.2.1 */
    DYMO_RM* updateRoutes(DYMO_RM * pkt);

    /** @brief dequeue packets to destinationAddress */
    void checkAndSendQueuedPkts(unsigned int destinationAddress, int prefix, unsigned int nextHopAddress);

    //===============================================================================
    // MEMBER VARIABLES
    //===============================================================================
    /** @brief pointer to the routing table */
    DYMO_RoutingTable *dymo_routingTable = nullptr;

    cMessage* timerMsg = nullptr; // timer self message, used for DYMO_Timer

    /** @brief runs after the node has lost its OwnSeqNum. When either ownSeqNumLossTimeout or ownSeqNumLossTimeoutMax expires, the resumes participation in DYMO */
    DYMO_Timer* ownSeqNumLossTimeout = nullptr;

    /** @brief runs after the node has lost its OwnSeqNum. When either ownSeqNumLossTimeout or ownSeqNumLossTimeoutMax expires, the resumes participation in DYMO */
    DYMO_Timer* ownSeqNumLossTimeoutMax = nullptr;

    /** @brief address of the node */
    unsigned int myAddr = 0;

    /** @brief sequence number of the node */
    unsigned int ownSeqNum = 0;

    /** @brief vector contains the RREQs sent, waiting for a reply */
    DYMO_OutstandingRREQList outstandingRREQList;

    /** @brief vector contains the queued data packets, waiting for a route */
    DYMO_DataQueue* queuedDataPackets = nullptr;

    /** @brief defines the routing block size */
    unsigned int rblockSize = 0;

    //  cPacket * rreq;
    //  cPacket * rerr;

    int totalPacketsSent = 0; /**< total number of packets sent down to lower layer */
    int totalBytesSent = 0; /**< total number of bytes sent down to lower layer */

    int statsRREQSent = 0; /**< number of generated DYMO RREQs */
    int statsRREPSent = 0; /**< number of generated DYMO RREPs */
    int statsRERRSent = 0; /**< number of generated DYMO RERRs */

    int statsRREQRcvd = 0; /**< number of consumed DYMO RREQs */
    int statsRREPRcvd = 0; /**< number of consumed DYMO RREPs */
    int statsRERRRcvd = 0; /**< number of consumed DYMO RERRs */

    int statsRREQFwd = 0; /**< number of forwarded (and processed) DYMO RREQs */
    int statsRREPFwd = 0; /**< number of forwarded (and processed) DYMO RREPs */
    int statsRERRFwd = 0; /**< number of forwarded (and processed) DYMO RERRs */

    int statsDYMORcvd = 0; /**< number of observed DYMO messages */

    simtime_t discoveryLatency;
    int disSamples = -1;
    simtime_t dataLatency;
    int dataSamples = -1;

    cOutVector discoveryDelayVec;
    cOutVector dataDelayVec;
    //cOutVector dataDelay2Vec;
    //cOutVector dataLoadVec;
    //cOutVector controlLoadVec;

    DYMO_TokenBucket* rateLimiterRREQ = nullptr;

    int RESPONSIBLE_ADDRESSES_PREFIX = -1; /**< NED parameter: netmask of network this DYMO router is responsible for, -1 for self only */
    const char* DYMO_INTERFACES = nullptr; /**< NED parameter: list of interfaces to run DYMO on, separated by a single space character */
    IPv4Address AUTOASSIGN_ADDRESS_BASE; /**< NED parameter: start of address range from which to automatically assign an address to the DYMO_INTERFACES */
    int ROUTE_AGE_MIN_TIMEOUT = -1;
    int ROUTE_AGE_MAX_TIMEOUT = -1;
    int ROUTE_NEW_TIMEOUT = -1;
    int ROUTE_USED_TIMEOUT = -1;
    int ROUTE_DELETE_TIMEOUT = -1;
    int MIN_HOPLIMIT = -1; /**< NED parameter: RREQs are first tried with this MsgHdr.HopLimit */
    int MAX_HOPLIMIT = -1; /**< NED parameter: MsgHdr.HopLimit for last RREQ, as well as other DYMO control messages */
    double RREQ_RATE_LIMIT = NaN; /**< NED parameter: maximum average RREQs per second (token bucket) */
    int RREQ_BURST_LIMIT = -1; /**< NED parameter: maximum RREQs in a burst (token bucket) */
    simtime_t RREQ_WAIT_TIME;
    unsigned int RREQ_TRIES = 0;
    int BUFFER_SIZE_PACKETS = -1; /**< NED configuration parameter: maximum number of queued packets, -1 for no limit */
    int BUFFER_SIZE_BYTES = -1; /**< NED configuration parameter: maximum total size of queued packets, -1 for no limit */

    virtual bool supportGetRoute() override {return false;}
    virtual uint32_t getRoute(const L3Address &, std::vector<L3Address> &add) override {return 0;};
    virtual bool getNextHop(const L3Address &, L3Address &add, int &iface, double &val) override {return false;}
    virtual void setRefreshRoute(const L3Address &destination, const L3Address & nextHop,bool isReverse) override {};
    virtual bool isProactive() override {return false;};
    virtual bool isOurType(cPacket * msg) override
    {
        if (dynamic_cast<DYMO_PacketBBMessage*>(msg))
            return true;
        else if (dynamic_cast<DYMO_Packet*>(msg))
            return true;
        else
            return false;
    };
    virtual bool getDestAddress(cPacket *, L3Address &) override {return false;};

    virtual void processLinkBreak(const cObject *details) override;
    void packetFailed(const IPv4Datagram *dgram);
    void rescheduleTimer();
};

} // namespace inetmanet

} // namespace inet

#endif

