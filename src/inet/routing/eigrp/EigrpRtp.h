//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef __INET_EIGRPRTP_H_
#define __INET_EIGRPRTP_H_

#include <omnetpp.h>

#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/routing/eigrp/EigrpDualStack.h"
#include "inet/routing/eigrp/messages/EigrpMsgReq.h"
#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"
#include "inet/routing/eigrp/tables/EigrpNeighborTable.h"
namespace inet {
namespace eigrp{
class EigrpRtp;

/**
 * Queue for storing requests for message sending.
 */
class EigrpRequestQueue: public cObject
{
  private:
    typedef std::list<EigrpMsgReq *> MessageQueue;

    MessageQueue reqQueue;          /**< Queue with requests waiting for sending (rel/unrel) */

    // Only for debugging
    //friend class EigrpRtp;

  public:
    virtual ~EigrpRequestQueue();

    void pushReq(EigrpMsgReq *req);
    EigrpMsgReq *findReqByIf(int ifaceId, bool sent = true);
    EigrpMsgReq *findReqByNeighbor(int neighId, bool sent = true);
    EigrpMsgReq *findUnrelReqByIf(int ifaceId);
    EigrpMsgReq *findReqBySeq(uint32_t seqNumber);
    EigrpMsgReq *removeReq(EigrpMsgReq *msgReq);
    void removeAllMsgsToIf(int ifaceId);
    void removeAllMsgsToNeigh(int ifaceId);
    int getNumReq() const { return reqQueue.size(); }

    void printInfo() const;
};

/**
 * Class represents Reliable Transport Protocol for reliable transmission of EIGRP messages.
 */
template <typename IPAddress>
class EigrpRtpT : public cSimpleModule
{
    struct NeighborInfo
    {
        int neighborId;
        int neighborIfaceId;
        uint32_t lastSeqNum;
        int numOfAck;
    };
    typedef std::list<EigrpMsgReq *> MessageQueue;

    const char *RTP_OUTPUT_GW;

    uint32_t seqNumber;             /**< Sequence number for reliable transport of messages */

    EigrpRequestQueue *requestQ;

    EigrpInterfaceTable *eigrpIft;
    EigrpNeighborTable<IPAddress> *eigrpNt; //TODO - verify! Should I use EigrpIpv4NeighborTable?

    /**
     * Process request for message sending.
     */
    void processRequest(cMessage *msg);
    /**
     * Process message header for ensuring reliable transmission.
     */
    void processHeader(cMessage *msg);

    /**
     * Schedule sending next reliable/unreliable message in transmission queue.
     */
    void scheduleNextMsg(int ifaceId);
    /**
     * Schedule new request for sending message.
     */
    void scheduleNewMsg(EigrpMsgReq *msgReq);
    /**
     * Send reliable/unreliable message
     */
    void sendMsg(EigrpMsgReq *msgReq);
    /**
     * Send reliable message.
     */
    void sendRelMsg(EigrpMsgReq *msgReq);
    /**
     * Send unreliable message.
     */
    void sendUnrelMsg(EigrpMsgReq *msgReq);
    /**
     * Delete request.
     */
    void discardMsg(EigrpMsgReq *msgReq);

    /**
     * Return informations about neighbor.
     */
    EigrpNeighbor<IPAddress> *getNeighborId(cMessage *msg);
    /**
     * Send message with specified acknowledge number to neighbor.
     */
    void acknowledgeMsg(int neighId, int ifaceId, uint32_t ackNum);

  protected:
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }

  public:
    EigrpRtpT();
    virtual ~EigrpRtpT();

};

class EigrpRtp : public EigrpRtpT<Ipv4Address>
{
//container class for IPv4RTP, must exist because of Define_Module()
public:
    virtual ~EigrpRtp() {};
};

#ifndef DISABLE_EIGRP_IPV6
class EigrpRtp6 : public EigrpRtpT<Ipv6Address>
{
//container class for IPv6RTP, must exist because of Define_Module()
public:
    virtual ~EigrpRtp6() {};
};
#endif /* DISABLE_EIGRP_IPV6 */
} //eigrp
} //inet
#endif
