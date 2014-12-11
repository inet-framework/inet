/*
 * ProbabilisticBroadcast.cc
 *
 *  Created on: Nov 4, 2008
 *      Author: Damien Piguet
 */
#include "inet/networklayer/probabilistic/ProbabilisticBroadcast.h"

#include <cassert>

#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/networklayer/common/SimpleNetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

using std::make_pair;
using std::endl;

Define_Module(ProbabilisticBroadcast);

long ProbabilisticBroadcast::id_counter = 0;

void ProbabilisticBroadcast::initialize(int stage)
{
    NetworkProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        broadcastPeriod = par("bcperiod");
        beta = par("beta");
        maxNbBcast = par("maxNbBcast");
        headerLength = par("headerLength");
        timeInQueueAfterDeath = par("timeInQueueAfterDeath");
        timeToLive = par("timeToLive");
        broadcastTimer = new cMessage("broadcastTimer");
        maxFirstBcastBackoff = par("maxFirstBcastBackoff");
        oneHopLatencies.setName("oneHopLatencies");
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        debugNbMessageKnown = 0;
        nbDataPacketsForwarded = 0;
        nbHops = 0;
    }
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        myNetwAddr = interfaceTable->getInterface(1)->getNetworkAddress();
    }
}

void ProbabilisticBroadcast::handleUpperPacket(cPacket *msg)
{
    ProbabilisticBroadcastDatagram *pkt;

    // encapsulate message in a network layer packet.
    pkt = check_and_cast<ProbabilisticBroadcastDatagram *>(encapsMsg(check_and_cast<cPacket *>(msg)));
    nbDataPacketsSent++;
    EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleUpperMsg(): Pkt ID = " << pkt->getId() << " TTL = " << pkt->getAppTtl() << endl;
    // submit packet for first insertion in queue.
    insertNewMessage(pkt, true);
}

void ProbabilisticBroadcast::handleLowerPacket(cPacket *msg)
{
    MACAddress macSrcAddr;
    ProbabilisticBroadcastDatagram *m = check_and_cast<ProbabilisticBroadcastDatagram *>(msg);
    IMACProtocolControlInfo *cInfo = check_and_cast<IMACProtocolControlInfo *>(m->removeControlInfo());
    m->setNbHops(m->getNbHops() + 1);
    macSrcAddr = cInfo->getSourceAddress();
    delete cInfo;
    ++nbDataPacketsReceived;
    nbHops = nbHops + m->getNbHops();
    oneHopLatencies.record(SIMTIME_DBL(simTime() - m->getTimestamp()));
    // oneHopLatency gives us an estimate of how long the message spent in the MAC queue of
    // its sender (compared to that, transmission delay is negligible). Use this value
    // to update the TTL of the message. Dump it if it is dead.
    //m->setAppTtl(m->getAppTtl().dbl() - oneHopLatency);
    if (    /*(m->getAppTtl() <= 0) || */ (messageKnown(m->getId()))) {
        // we got this message already, ignore it.
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleLowerMsg(): Dead or Known message ID=" << m->getId() << " from node "
           << macSrcAddr << " TTL = " << m->getAppTtl() << endl;
        delete m;
    }
    else {
        if (debugMessageKnown(m->getId())) {
            ++debugNbMessageKnown;
            EV << "PBr: " << simTime() << " n" << myNetwAddr << " ERROR Message should be known TTL= " << m->getAppTtl() << endl;
        }
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleLowerMsg(): Unknown message ID=" << m->getId() << " from node "
           << macSrcAddr << endl;
        // Unknown message. Insert message in queue with random backoff broadcast delay.
        // Because we got the message from lower layer, we need to create and add a new
        // control info with the MAC destination address = broadcast address.
        setDownControlInfo(m, MACAddress::BROADCAST_ADDRESS);
        // before inserting message, update source address (for this hop, not the initial source)
        m->setSrcAddr(myNetwAddr);
        insertNewMessage(m);

        // until a subscription mechanism is implemented, duplicate and pass all received packets
        // to the application layer who will be able to compute statistics.
        // TODO: implement an application subscription mechanism.
        if (true) {
            int protocol = m->getTransportProtocol();
            ProbabilisticBroadcastDatagram *mCopy = check_and_cast<ProbabilisticBroadcastDatagram *>(m->dup());
            sendUp(decapsMsg(mCopy), protocol);
        }
    }
}

void ProbabilisticBroadcast::handleSelfMessage(cMessage *msg)
{
    if (msg == broadcastTimer) {
        tMsgDesc *msgDesc;
        ProbabilisticBroadcastDatagram *pkt;

        // called method pops the first message from the message queue and
        // schedules the message timer for the next one. The message is embedded
        // into a container of type tMsgDesc.
        msgDesc = popFirstMessageUpdateQueue();
        pkt = msgDesc->pkt;
        // if the packet is alive, duplicate it and insert the copy in the queue,
        // then perform a broadcast attempt.
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleSelfMsg(): Message ID= " << pkt->getId() << " TTL= " << pkt->getAppTtl() << endl;
        if (pkt->getAppTtl() > 0) {
            // check if we are allowed to re-transmit the message on more time.
            if (msgDesc->nbBcast < maxNbBcast) {
                ProbabilisticBroadcastDatagram *pktCopy;
                bool sendForSure = msgDesc->initialSend;

                // duplicate packet and insert the copy in the queue.
                // two possibilities: the packet will be alive at next
                // broadcast period => insert it with delay = broadcastPeriod.
                // Or the packet will be dead at next broadcast period (TTL <= broadcastPeriod)
                // => insert it with delay = TTL. So when the copy will be popped out of the
                // queue, it will be considered as dead and discarded.
                pktCopy = pkt->dup();
                // control info is not duplicated with the message, so we have to re-create one here.
                setDownControlInfo(pktCopy, MACAddress::BROADCAST_ADDRESS);
                // it the copy that is re-inserted into the queue so update the container accordingly
                msgDesc->pkt = pktCopy;
                // increment nbBcast field of the descriptor because at this point, it is sure that
                // the message will go through one more broadcast attempt.
                msgDesc->nbBcast++;
                // for sure next broadcast attempt will not be the initial one.
                msgDesc->initialSend = false;
                // if msg TTL > broadcast period, the message will be broadcasted one more
                // time, insert it with delay = broadcast period. Otherwise, the message
                // will be dead at next broadcast attempt. Keep it in the list with
                // delay = TTL + timeInQueueAfterDeath. insertMessage() will update its
                // TTL to -timeInQueueAfterDeath, a negative value. That way, the message
                // is known to the system, de-synchronization between copies of the same message
                // is therefore handled and when the message will be popped out, its TTL will
                // be smaller than zero, thus the message will be discarded, not broadcasted.
                if (pktCopy->getAppTtl() > broadcastPeriod)
                    insertMessage(broadcastPeriod, msgDesc);
                else
                    insertMessage(pktCopy->getAppTtl() + timeInQueueAfterDeath, msgDesc);
                // broadcast the message with probability beta
                if (sendForSure) {
                    EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Send packet down for sure." << endl;
                    pkt->setTimestamp();
                    sendDown(pkt);
                    ++nbDataPacketsForwarded;
                }
                else {
                    if (bernoulli(beta)) {
                        EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Bernoulli test result: TRUE. Send packet down." << endl;
                        pkt->setTimestamp();
                        sendDown(pkt);
                        ++nbDataPacketsForwarded;
                    }
                    else {
                        EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Bernoulli test result: FALSE" << endl;
                        delete pkt;
                    }
                }
            }
            else {
                // we can't re-transmit the message because maxNbBcast is reached.
                // re-insert-it in the queue with delay = TTL so that its ID is still
                // known by the system.
                EV << "PBr: " << simTime() << " n" << myNetwAddr << "     maxNbBcast reached." << endl;
                insertMessage(pkt->getAppTtl() + timeInQueueAfterDeath, msgDesc);
            }
        }
        else {
            EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Message TTL zero, discard." << endl;
            delete msgDesc;
            delete pkt;
        }
    }
    else {
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " Received unexpected self message" << endl;
    }
}

void ProbabilisticBroadcast::finish()
{
    EV << "PBr: " << simTime() << " n" << myNetwAddr << " finish()" << endl;
    cancelAndDelete(broadcastTimer);
    // if some messages are still in the queue, delete them.
    while (!msgQueue.empty()) {
        auto pos = msgQueue.begin();
        tMsgDesc *msgDesc = pos->second;
        msgQueue.erase(pos);
        delete msgDesc->pkt;
        delete msgDesc;
    }
    recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
    recordScalar("debugNbMessageKnown", debugNbMessageKnown);
    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    if (nbDataPacketsReceived > 0) {
        recordScalar("meanNbHops", (double)nbHops / (double)nbDataPacketsReceived);
    }
    else {
        recordScalar("meanNbHops", 0);
    }
}

bool ProbabilisticBroadcast::messageKnown(unsigned int msgId)
{
    auto pos = knownMsgIds.find(msgId);
    return pos != knownMsgIds.end();
}

bool ProbabilisticBroadcast::debugMessageKnown(unsigned int msgId)
{
    auto pos = debugMsgIdSet.find(msgId);
    return pos != debugMsgIdSet.end();
}

void ProbabilisticBroadcast::insertMessage(simtime_t_cref bcastDelay, tMsgDesc *msgDesc)
{
    simtime_t bcastTime = simTime() + bcastDelay;

    EV << "PBr: " << simTime() << " n" << myNetwAddr << "         insertMessage() bcastDelay = " << bcastDelay << " Msg ID = " << msgDesc->pkt->getId() << endl;
    // update TTL field of the message to the value it will have when taken out of the list
    msgDesc->pkt->setAppTtl(msgDesc->pkt->getAppTtl() - bcastDelay);
    // insert message ID in ID list.
    knownMsgIds.insert(msgDesc->pkt->getId());
    // insert key value pair <broadcast time, pointer to message> in message queue.
    auto pos = msgQueue.insert(make_pair(bcastTime, msgDesc));
    // if the message has been inserted in the front of the list, it means that it
    // will be the next message to be broadcasted, therefore we have to re-schedule
    // the broadcast timer to the message's broadcast instant.
    if (pos == msgQueue.begin()) {
        EV << "PBr: " << simTime() << " n" << myNetwAddr << "         message inserted in the front, reschedule it." << endl;
        cancelEvent(broadcastTimer);
        scheduleAt(bcastTime, broadcastTimer);
    }
}

ProbabilisticBroadcast::tMsgDesc *ProbabilisticBroadcast::popFirstMessageUpdateQueue(void)
{
    tMsgDesc *msgDesc;

    // get first message.
    ASSERT(!msgQueue.empty());
    auto pos = msgQueue.begin();
    msgDesc = pos->second;
    // remove first message from message queue and from ID list
    msgQueue.erase(pos);
    knownMsgIds.erase(msgDesc->pkt->getId());
    EV << "PBr: " << simTime() << " n" << myNetwAddr << "         pop(): just popped msg " << msgDesc->pkt->getId() << endl;
    if (!msgQueue.empty()) {
        // schedule broadcast of new first message
        EV << "PBr: " << simTime() << " n" << myNetwAddr << "         pop(): schedule next message." << endl;
        pos = msgQueue.begin();
        scheduleAt(pos->first, broadcastTimer);
    }
    return msgDesc;
}

cPacket *ProbabilisticBroadcast::encapsMsg(cPacket *msg)
{
    ProbabilisticBroadcastDatagram *pkt = new ProbabilisticBroadcastDatagram(msg->getName());
    cObject *controlInfo = msg->removeControlInfo();
    INetworkProtocolControlInfo *networkControlInfo = check_and_cast<INetworkProtocolControlInfo *>(controlInfo);
    L3Address broadcastAddress = myNetwAddr.getAddressType()->getBroadcastAddress();

    ASSERT(networkControlInfo);
    pkt->setByteLength(headerLength);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(broadcastAddress);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setFinalDestAddr(broadcastAddress);
    pkt->setAppTtl(timeToLive);
    pkt->setId(getNextID());
    pkt->setTransportProtocol(networkControlInfo->getTransportProtocol());

    setDownControlInfo(pkt, MACAddress::BROADCAST_ADDRESS);
    //encapsulate the application packet
    pkt->encapsulate(msg);

    // clean-up
    delete controlInfo;

    return pkt;
}

void ProbabilisticBroadcast::insertNewMessage(ProbabilisticBroadcastDatagram *pkt, bool iAmInitialSender)
{
    simtime_t ttl = pkt->getAppTtl();

    if (ttl > 0) {
        simtime_t bcastDelay;
        tMsgDesc *msgDesc;

        // insert packet in queue with delay in [0, min(TTL, broadcast period)].
        // since the insertion schedules the message for its first broadcast attempt,
        // we use a uniform random back-off taken between now and the broadcast delay
        // to avoid having all nodes in the neighborhood forward the packet at the same
        // time. Backoffs used at MAC layer are thought to be too short.
        if (broadcastPeriod < maxFirstBcastBackoff)
            bcastDelay = broadcastPeriod;
        else
            bcastDelay = maxFirstBcastBackoff;
        if (bcastDelay > ttl)
            bcastDelay = ttl;
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " insertNewMessage(): insert packet " << pkt->getId() << " with delay "
           << bcastDelay << endl;
        // create container for message and initialize container's values.
        msgDesc = new tMsgDesc;
        msgDesc->pkt = pkt;
        msgDesc->nbBcast = 0;    // so far, pkt has been forwarded zero times.
        msgDesc->initialSend = iAmInitialSender;
        debugMsgIdSet.insert(pkt->getId());
        insertMessage(uniform(0, bcastDelay), msgDesc);
    }
    else {
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " insertNewMessage(): got new packet with TTL = 0." << endl;
        delete pkt;
    }
}

cPacket *ProbabilisticBroadcast::decapsMsg(ProbabilisticBroadcastDatagram *msg)
{
    cPacket *m = msg->decapsulate();
    SimpleNetworkProtocolControlInfo *const controlInfo = new SimpleNetworkProtocolControlInfo();
    controlInfo->setSourceAddress(msg->getSrcAddr());
    controlInfo->setProtocol(msg->getTransportProtocol());
    m->setControlInfo(controlInfo);
    delete msg;
    return m;
}

/**
 * Attaches a "control info" structure (object) to the down message pMsg.
 */
cObject *ProbabilisticBroadcast::setDownControlInfo(cMessage *const pMsg, const MACAddress& pDestAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setDest(pDestAddr);
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}

} // namespace inet

