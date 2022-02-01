//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

//#include "inet/networklayer/contract/ipv4/Ipv4ControlInfo.h"
//#include "inet/networklayer/contract/ipv6/Ipv6ControlInfo.h"
#include "inet/routing/eigrp/EigrpRtp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
namespace inet {
namespace eigrp {
#define EIGRP_RTP_DEBUG

Define_Module(EigrpRtp);

#ifndef DISABLE_EIGRP_IPV6
Define_Module(EigrpRtp6);
#endif /* DISABLE_EIGRP_IPV6 */

/*
 * TODO resit pad rozhrani -> smazani zprav pres dane rozhrani (bud receive notification (asi lepsi) nebo kontrolovat existenci rozhrani)
 *  - pad souseda se ale musi kontrolovat!
 */

namespace eigrpRtp {

// User message codes
enum UserMsgCodes {
    M_OK = 0,                             // no message
    M_UPDATE_SEND = EIGRP_UPDATE_MSG,     // send Update message
    M_REQUEST_SEND = EIGRP_REQUEST_MSG,   // send Request message
    M_QUERY_SEND = EIGRP_QUERY_MSG,       // send Query message
    M_REPLY_SEND = EIGRP_REPLY_MSG,       // send Query message
    M_HELLO_SEND = EIGRP_HELLO_MSG,       // send Hello message
};

// User messages
const char *UserMsgs[] =
{
    // M_OK
    "OK",
    // M_UPDATE_SEND
    "Update",
    // M_REQUEST_SEND
    "Request",
    // M_QUERY_SEND
    "Query",
    // M_REPLY_SEND
    "Reply",
    // M_HELLO_SEND
    "Hello",
};
}; // end of namespace eigrp

std::ostream& operator<<(std::ostream& os, const EigrpMsgReq& req)
{
    os << "Type:" << eigrpRtp::UserMsgs[req.getOpcode()];
    os << "  destIF:" << req.getDestInterface();
    os << "  destNeighbor:" << req.getDestNeighbor();
    os << "  seqNumber:" << req.getSeqNumber();
    os << "  numberOfAck:" << req.getNumOfAck();
    return os;
}

void EigrpRequestQueue::printInfo() const
{
    MessageQueue::const_iterator it;
    EigrpMsgReq *req;

    EV_DEBUG << "EIGRP RTP: content of request queue:" << endl;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        req = *it;

        EV_DEBUG << "  Type:" << eigrpRtp::UserMsgs[req->getOpcode()];
        EV_DEBUG << "  destIF:" << req->getDestInterface();
        EV_DEBUG << "  destNeighbor:" << req->getDestNeighbor();
        EV_DEBUG << "  seqNumber:" << req->getSeqNumber();
        EV_DEBUG << "  ackNumber:" << req->getAckNumber();
        EV_DEBUG << "  numberOfAck:" << req->getNumOfAck() << endl;
    }
}

EigrpRequestQueue::~EigrpRequestQueue()
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        delete *it;
    }
}

/**
 * Get first rel/unrel message with given interface ID from request Queue.
 * @param sent If false search only for messages waiting for send (not actually sent).
 */
EigrpMsgReq *EigrpRequestQueue::findReqByIf(int ifaceId, bool sent)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it)->getDestInterface() == ifaceId) {
            if (sent) // Get first message
                return *it;
            else if ((*it)->getNumOfAck() == 0)
                return *it;
        }
    }
    return nullptr;
}

EigrpMsgReq *EigrpRequestQueue::findReqByNeighbor(int neighId, bool sent)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it)->getDestNeighbor() == neighId) {
            if (sent) // Get first message
                return *it;
            else if ((*it)->getNumOfAck() == 0)
                return *it;
        }
    }
    return nullptr;
}

/**
 * Get first unrel message with given interface ID from request Queue.
 */
EigrpMsgReq *EigrpRequestQueue::findUnrelReqByIf(int ifaceId)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it)->getDestInterface() == ifaceId && !(*it)->isMsgReliable()) {
            return *it;
        }
    }
    return nullptr;
}

EigrpMsgReq *EigrpRequestQueue::findReqBySeq(uint32_t seqNumber)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it)->getSeqNumber() == seqNumber)
            return *it;
    }
    return nullptr;
}

void EigrpRequestQueue::pushReq(EigrpMsgReq *req)
{
    reqQueue.push_back(req);
#ifdef EIGRP_RTP_DEBUG
//    printInfo();
#endif
}

EigrpMsgReq *EigrpRequestQueue::removeReq(EigrpMsgReq *msgReq)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end(); it++) {
        if ((*it) == msgReq) {
            reqQueue.erase(it);
#ifdef EIGRP_RTP_DEBUG
//            printInfo();
#endif
            return msgReq;
        }
    }
    return nullptr;
}

/**
 * Delete all messages with given destination interface ID (fall of interface,...).
 */
void EigrpRequestQueue::removeAllMsgsToIf(int ifaceId)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end();) {
        if ((*it)->getDestInterface() == ifaceId) {
            delete *it;
            it = reqQueue.erase(it);
        }
        else
            ++it;
    }
}

/**
 * Delete all messages with given destination neighbor ID (end of neighborship,...).
 */
void EigrpRequestQueue::removeAllMsgsToNeigh(int neighborId)
{
    MessageQueue::iterator it;
    for (it = reqQueue.begin(); it != reqQueue.end();) {
        if ((*it)->getDestNeighbor() == neighborId) {
            delete *it;
            it = reqQueue.erase(it);
        }
        else
            ++it;
    }
}

template<typename IPAddress>
EigrpRtpT<IPAddress>::EigrpRtpT()
{
    RTP_OUTPUT_GW = "pdmOut";
}

template<typename IPAddress>
EigrpRtpT<IPAddress>::~EigrpRtpT()
{
    delete requestQ;
}

template<>
void EigrpRtpT<Ipv4Address>::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        seqNumber = 1;

//        this->eigrpIft = EigrpIfTableAccess().get();
//        this->eigrpNt = Eigrpv4NeighTableAccess().get();
        eigrpIft = check_and_cast<EigrpInterfaceTable *>(getModuleByPath("^.eigrpInterfaceTable"));
        eigrpNt = check_and_cast<EigrpIpv4NeighborTable *>(getModuleByPath("^.eigrpIpv4NeighborTable"));

        requestQ = new EigrpRequestQueue();

//        WATCH_PTRLIST(requestQ->reqQueue);
    }
}

#ifndef DISABLE_EIGRP_IPV6
template<>
void EigrpRtpT<Ipv6Address>::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        seqNumber = 1;

//        this->eigrpIft = EigrpIfTable6Access().get();
//        this->eigrpNt = Eigrpv6NeighTableAccess().get();
        eigrpIft = check_and_cast<EigrpInterfaceTable *>(getModuleByPath("^.eigrpInterfaceTable6"));
        eigrpNt = check_and_cast<EigrpIpv6NeighborTable *>(getModuleByPath("^.eigrpIpv6NeighborTable"));
        requestQ = new EigrpRequestQueue();

//        WATCH_PTRLIST(requestQ->reqQueue);
    }
}

#endif /* DISABLE_EIGRP_IPV6 */

template<typename IPAddress>
void EigrpRtpT<IPAddress>::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage()) { // Timer
    }
    else {
        if (dynamic_cast<EigrpMsgReq *>(msg) != nullptr) { // EIGRP message request
            processRequest(msg);

            // Do not delete msg
        }
        else { // Process EIGRP header
            processHeader(msg);

            delete msg;
        }
    }
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::processRequest(cMessage *msg)
{
    EigrpMsgReq *msgReq = check_and_cast<EigrpMsgReq *>(msg);

    scheduleNewMsg(msgReq);
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::processHeader(cMessage *msg)
{
    Packet *pk = check_and_cast<Packet *>(msg);
    const auto& header = pk->peekAtFront<EigrpMessage>();
    uint32_t seqNumNeigh; // Sequence number of neighbor
    uint32_t ackNum; // Acknowledge number
    EigrpMsgReq *msgReq = nullptr;
    int numOfAck;
    EigrpNeighbor<IPAddress> *neigh = nullptr;
    EigrpInterface *eigrpIface = nullptr;

    if ((neigh = getNeighborId(msg)) == nullptr)
        return;

//    EV_DEBUG << "EIGRP RTP: received " << eigrpRtp::UserMsgs[header->getOpcode()] << " message for processing" << endl;
    //header->getOpcode() == EIGRP_UPDATE_MSG
    seqNumNeigh = header->getSeqNum();
    ackNum = header->getAckNum();

    if (ackNum != 0) { // Acknowledge of message
        if ((msgReq = requestQ->findReqBySeq(ackNum)) != nullptr && neigh->getAck() == ackNum) { // Record ack
            neigh->setAck(0);
            numOfAck = msgReq->getNumOfAck();
            msgReq->setNumOfAck(--numOfAck);

            if (numOfAck == 0) { // All acknowledges received
                eigrpIface = eigrpIft->findInterfaceById(neigh->getIfaceId());
                eigrpIface->decPendingMsgs();

                // Delete request
                requestQ->removeReq(msgReq);
                delete msgReq;
                msgReq = nullptr;

                scheduleNextMsg(neigh->getIfaceId());
            }
        }
        // else do nothing (wrong message ack)
    }

    if (seqNumNeigh != 0) { // Received message must be acknowledged
        // Store last sequence number from neighbor
        neigh->setSeqNumber(seqNumNeigh);

        acknowledgeMsg(neigh->getNeighborId(), neigh->getIfaceId(), seqNumNeigh);
    }
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::acknowledgeMsg(int neighId, int ifaceId, uint32_t ackNum)
{
    EigrpMsgReq *msgReq = nullptr;

    /*if ((msgReq = requestQ->findReqByIf(ifaceId, false)) != nullptr && msgReq->getDestNeighbor() == neighId)
    { // Use scheduled message as acknowledge
        EV_DEBUG << "EIGRP RTP: do not create Ack message, use existing message to neighbor " << neighId << endl;
        msgReq->setAckNumber(ackNum);
    }
    else*/
    // Create Ack message and send it
    msgReq = new EigrpMsgReq("Ack");
    msgReq->setOpcode(EIGRP_HELLO_MSG);
    msgReq->setDestNeighbor(neighId);
    msgReq->setDestInterface(ifaceId);
    msgReq->setAckNumber(ackNum);

    scheduleNewMsg(msgReq);
}

template<>
EigrpNeighbor<Ipv4Address> *EigrpRtpT<Ipv4Address>::getNeighborId(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);
    Ipv4Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv4();

    return eigrpNt->findNeighbor(srcAddr);
}

#ifndef DISABLE_EIGRP_IPV6
template<>
EigrpNeighbor<Ipv6Address> *EigrpRtpT<Ipv6Address>::getNeighborId(cMessage *msg)
{
    Packet *packet = check_and_cast<Packet *>(msg);
    Ipv6Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress().toIpv6();

    return eigrpNt->findNeighbor(srcAddr);
}

#endif /* DISABLE_EIGRP_IPV6 */

template<typename IPAddress>
void EigrpRtpT<IPAddress>::scheduleNewMsg(EigrpMsgReq *msgReq)
{
    requestQ->pushReq(msgReq);

    EV_DEBUG << "EIGRP RTP: enqueue " << eigrpRtp::UserMsgs[msgReq->getOpcode()];
    if (msgReq->getOpcode() == EIGRP_HELLO_MSG && msgReq->getAckNumber() != 0)
        EV_DEBUG << " (ack) ";
    EV_DEBUG << " on IF: " << msgReq->getDestInterface() << endl;

    scheduleNextMsg(msgReq->getDestInterface());
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::scheduleNextMsg(int ifaceId)
{
    EigrpMsgReq *msgReq = nullptr;
    EigrpInterface *eigrpIface = eigrpIft->findInterfaceById(ifaceId);

    if (eigrpIface == nullptr) {
        requestQ->removeAllMsgsToIf(ifaceId);
        return;
    }

    if (eigrpIface->getPendingMsgs() == 0) { // Try to send first rel/unrel message
        if ((msgReq = requestQ->findReqByIf(ifaceId)) != nullptr) {
            ASSERT(msgReq->getNumOfAck() == 0);
            sendMsg(msgReq);
        }
    }
    else { // Try to send first unrel message
        if ((msgReq = requestQ->findUnrelReqByIf(ifaceId)) != nullptr) {
            sendUnrelMsg(msgReq);
        }
    }
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::discardMsg(EigrpMsgReq *msgReq)
{
    EV_DEBUG << "EIGRP RTP: discard message " << eigrpRtp::UserMsgs[msgReq->getOpcode()] << endl;
    requestQ->removeReq(msgReq);
    delete msgReq;

    scheduleNextMsg(msgReq->getDestInterface());
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::sendUnrelMsg(EigrpMsgReq *msgReq)
{
    int ifaceId = msgReq->getDestInterface();

    requestQ->removeReq(msgReq);
    send(msgReq, RTP_OUTPUT_GW); /* Do not duplicate EigrpMsgReq */
    // Schedule next waiting message
    scheduleNextMsg(ifaceId);
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::sendMsg(EigrpMsgReq *msgReq)
{
    if (msgReq->isMsgReliable()) { // Reliable
        sendRelMsg(msgReq);
    }
    else { // Unreliable
        sendUnrelMsg(msgReq);
    }
}

template<typename IPAddress>
void EigrpRtpT<IPAddress>::sendRelMsg(EigrpMsgReq *msgReq)
{
    NeighborInfo info;
    EigrpMsgReq *msgToSend = nullptr;
    EigrpNeighbor<IPAddress> *neigh = nullptr;
    EigrpInterface *eigrpIface = nullptr;

    info.neighborId = msgReq->getDestNeighbor();
    info.neighborIfaceId = msgReq->getDestInterface();

    if (msgReq->getSeqNumber() == 0) { // Add sequence number
        msgReq->setSeqNumber(seqNumber);
        seqNumber++;
    }

    if (info.neighborId != 0) { // Unicast
        if ((neigh = eigrpNt->findNeighborById(info.neighborId)) == nullptr) {
            requestQ->removeAllMsgsToNeigh(info.neighborId);
            discardMsg(msgReq);
            return;
        }
        info.numOfAck = 1;
        neigh->setAck(msgReq->getSeqNumber());
        info.lastSeqNum = neigh->getSeqNumber();
    }
    else { // Multicast
        info.numOfAck = eigrpNt->setAckOnIface(info.neighborIfaceId, msgReq->getSeqNumber());
        info.lastSeqNum = 0; // Multicast message can not be an ack
    }

    eigrpIface = eigrpIft->findInterfaceById(info.neighborIfaceId);
    eigrpIface->incPendingMsgs();

    // Add last sequence number from neighbor
    msgReq->setAckNumber(info.lastSeqNum);
    msgReq->setNumOfAck(info.numOfAck);

    // Request and message must be duplicated (PDM deletes request and sends message)
    msgToSend = msgReq->dup();
    send(msgToSend, RTP_OUTPUT_GW);

    scheduleNextMsg(info.neighborIfaceId);
}

template class EigrpRtpT<Ipv4Address>;

#ifndef DISABLE_EIGRP_IPV6
template class EigrpRtpT<Ipv6Address>;
#endif /* DISABLE_EIGRP_IPV6 */
} // eigrp
} // inet

