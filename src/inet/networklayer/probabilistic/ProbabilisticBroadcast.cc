/*
 * ProbabilisticBroadcast.cc
 *
 *  Created on: Nov 4, 2008
 *      Author: Damien Piguet
 */

#include "inet/networklayer/probabilistic/ProbabilisticBroadcast.h"

#include <cassert>

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/stlutils.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

using std::make_pair;
using std::endl;

Define_Module(ProbabilisticBroadcast);

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
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
            interfaceTable->getInterface(i)->setHasModulePathAddress(true);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        if (auto ie = interfaceTable->findFirstNonLoopbackInterface())
            myNetwAddr = ie->getNetworkAddress();
        else
            throw cRuntimeError("No non-loopback interface found!");
    }
}

void ProbabilisticBroadcast::handleUpperPacket(Packet *packet)
{
    // encapsulate message in a network layer packet.
    encapsulate(packet);
    auto macHeader = packet->peekAtFront<ProbabilisticBroadcastHeader>();
    nbDataPacketsSent++;
    EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleUpperMsg(): Pkt ID = " << macHeader->getId() << " TTL = " << macHeader->getAppTtl() << endl;
    // submit packet for first insertion in queue.
    insertNewMessage(packet, true);
}

void ProbabilisticBroadcast::handleLowerPacket(Packet *packet)
{
    MacAddress macSrcAddr;
    auto macHeader = dynamicPtrCast<ProbabilisticBroadcastHeader>(packet->popAtFront<ProbabilisticBroadcastHeader>()->dupShared());
    packet->trim();
    auto macAddressInd = packet->getTag<MacAddressInd>();
    macHeader->setNbHops(macHeader->getNbHops() + 1);
    macSrcAddr = macAddressInd->getSrcAddress();
    delete packet->removeControlInfo();
    ++nbDataPacketsReceived;
    nbHops = nbHops + macHeader->getNbHops();
    oneHopLatencies.record(SIMTIME_DBL(simTime() - packet->getTimestamp()));
    // oneHopLatency gives us an estimate of how long the message spent in the MAC queue of
    // its sender (compared to that, transmission delay is negligible). Use this value
    // to update the TTL of the message. Dump it if it is dead.
//    m->setAppTtl(m->getAppTtl().dbl() - oneHopLatency);
    if (/*(m->getAppTtl() <= 0) || */ (messageKnown(macHeader->getId()))) {
        // we got this message already, ignore it.
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleLowerMsg(): Dead or Known message ID=" << macHeader->getId() << " from node "
           << macSrcAddr << " TTL = " << macHeader->getAppTtl() << endl;
        delete packet;
    }
    else {
        if (debugMessageKnown(macHeader->getId())) {
            ++debugNbMessageKnown;
            EV << "PBr: " << simTime() << " n" << myNetwAddr << " ERROR Message should be known TTL= " << macHeader->getAppTtl() << endl;
        }
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleLowerMsg(): Unknown message ID=" << macHeader->getId() << " from node "
           << macSrcAddr << endl;
        // Unknown message. Insert message in queue with random backoff broadcast delay.
        // Because we got the message from lower layer, we need to create and add a new
        // control info with the MAC destination address = broadcast address.
        setDownControlInfo(packet, MacAddress::BROADCAST_ADDRESS);
        // before inserting message, update source address (for this hop, not the initial source)
        macHeader->setSrcAddr(myNetwAddr);
        packet->insertAtFront(macHeader);
        insertNewMessage(packet);

        // until a subscription mechanism is implemented, duplicate and pass all received packets
        // to the application layer who will be able to compute statistics.
        // TODO implement an application subscription mechanism.
        if (true) {
            auto mCopy = packet->dup();
            decapsulate(mCopy);
            sendUp(mCopy);
        }
    }
}

void ProbabilisticBroadcast::handleSelfMessage(cMessage *msg)
{
    if (msg == broadcastTimer) {
        tMsgDesc *msgDesc;
        // called method pops the first message from the message queue and
        // schedules the message timer for the next one. The message is embedded
        // into a container of type tMsgDesc.
        msgDesc = popFirstMessageUpdateQueue();
        auto packet = msgDesc->pkt;
        auto macHeader = packet->peekAtFront<ProbabilisticBroadcastHeader>();
        // if the packet is alive, duplicate it and insert the copy in the queue,
        // then perform a broadcast attempt.
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " handleSelfMsg(): Message ID= " << macHeader->getId() << " TTL= " << macHeader->getAppTtl() << endl;
        if (macHeader->getAppTtl() > 0) {
            // check if we are allowed to re-transmit the message on more time.
            if (msgDesc->nbBcast < maxNbBcast) {
                bool sendForSure = msgDesc->initialSend;

                // duplicate packet and insert the copy in the queue.
                // two possibilities: the packet will be alive at next
                // broadcast period => insert it with delay = broadcastPeriod.
                // Or the packet will be dead at next broadcast period (TTL <= broadcastPeriod)
                // => insert it with delay = TTL. So when the copy will be popped out of the
                // queue, it will be considered as dead and discarded.
                auto packetCopy = packet->dup();
                auto macHeaderCopy = packetCopy->peekAtFront<ProbabilisticBroadcastHeader>();
                // control info is not duplicated with the message, so we have to re-create one here.
                setDownControlInfo(packetCopy, MacAddress::BROADCAST_ADDRESS);
                // it the copy that is re-inserted into the queue so update the container accordingly
                msgDesc->pkt = packetCopy;
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
                if (macHeaderCopy->getAppTtl() > broadcastPeriod)
                    insertMessage(broadcastPeriod, msgDesc);
                else
                    insertMessage(macHeaderCopy->getAppTtl() + timeInQueueAfterDeath, msgDesc);
                // broadcast the message with probability beta
                if (sendForSure) {
                    EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Send packet down for sure." << endl;
                    packet->setTimestamp();
                    sendDown(packet);
                    ++nbDataPacketsForwarded;
                }
                else {
                    if (bernoulli(beta)) {
                        EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Bernoulli test result: TRUE. Send packet down." << endl;
                        packet->setTimestamp();
                        sendDown(packet);
                        ++nbDataPacketsForwarded;
                    }
                    else {
                        EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Bernoulli test result: FALSE" << endl;
                        delete packet;
                    }
                }
            }
            else {
                // we can't re-transmit the message because maxNbBcast is reached.
                // re-insert-it in the queue with delay = TTL so that its ID is still
                // known by the system.
                EV << "PBr: " << simTime() << " n" << myNetwAddr << "     maxNbBcast reached." << endl;
                insertMessage(macHeader->getAppTtl() + timeInQueueAfterDeath, msgDesc);
            }
        }
        else {
            EV << "PBr: " << simTime() << " n" << myNetwAddr << "     Message TTL zero, discard." << endl;
            delete msgDesc;
            delete packet;
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
    return contains(knownMsgIds, msgId);
}

bool ProbabilisticBroadcast::debugMessageKnown(unsigned int msgId)
{
    return contains(debugMsgIdSet, msgId);
}

void ProbabilisticBroadcast::insertMessage(simtime_t_cref bcastDelay, tMsgDesc *msgDesc)
{
    simtime_t bcastTime = simTime() + bcastDelay;

    EV << "PBr: " << simTime() << " n" << myNetwAddr << "         insertMessage() bcastDelay = " << bcastDelay << " Msg ID = " << msgDesc->pkt->getId() << endl;
    // update TTL field of the message to the value it will have when taken out of the list
    msgDesc->pkt->trim();
    auto macHeader = msgDesc->pkt->removeAtFront<ProbabilisticBroadcastHeader>();
    macHeader->setAppTtl(macHeader->getAppTtl() - bcastDelay);
    msgDesc->pkt->insertAtFront(macHeader);
    // insert message ID in ID list.
    knownMsgIds.insert(macHeader->getId());
    // insert key value pair <broadcast time, pointer to message> in message queue.
    auto pos = msgQueue.insert(make_pair(bcastTime, msgDesc));
    // if the message has been inserted in the front of the list, it means that it
    // will be the next message to be broadcasted, therefore we have to re-schedule
    // the broadcast timer to the message's broadcast instant.
    if (pos == msgQueue.begin()) {
        EV << "PBr: " << simTime() << " n" << myNetwAddr << "         message inserted in the front, reschedule it." << endl;
        rescheduleAt(bcastTime, broadcastTimer);
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

void ProbabilisticBroadcast::encapsulate(Packet *packet)
{
    auto pkt = makeShared<ProbabilisticBroadcastHeader>(); // TODO msg->getName());
    cObject *controlInfo = packet->removeControlInfo();
    L3Address broadcastAddress = myNetwAddr.getAddressType()->getBroadcastAddress();

    pkt->setChunkLength(B(headerLength));
    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(broadcastAddress);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setFinalDestAddr(broadcastAddress);
    pkt->setAppTtl(timeToLive);
    pkt->setId(getNextID());
    pkt->setProtocol(packet->getTag<PacketProtocolTag>()->getProtocol());
    pkt->setPayloadLengthField(packet->getDataLength());
    // clean-up
    delete controlInfo;

    // encapsulate the application packet
    packet->insertAtFront(pkt);

    setDownControlInfo(packet, MacAddress::BROADCAST_ADDRESS);
}

void ProbabilisticBroadcast::insertNewMessage(Packet *packet, bool iAmInitialSender)
{
    auto macHeader = packet->peekAtFront<ProbabilisticBroadcastHeader>();
    simtime_t ttl = macHeader->getAppTtl();

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
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " insertNewMessage(): insert packet " << macHeader->getId() << " with delay "
           << bcastDelay << endl;
        // create container for message and initialize container's values.
        msgDesc = new tMsgDesc;
        msgDesc->pkt = packet;
        msgDesc->nbBcast = 0; // so far, pkt has been forwarded zero times.
        msgDesc->initialSend = iAmInitialSender;
        debugMsgIdSet.insert(macHeader->getId());
        insertMessage(uniform(0, bcastDelay), msgDesc);
    }
    else {
        EV << "PBr: " << simTime() << " n" << myNetwAddr << " insertNewMessage(): got new packet with TTL = 0." << endl;
        delete packet;
    }
}

void ProbabilisticBroadcast::decapsulate(Packet *packet)
{
    auto networkHeader = packet->popAtFront<ProbabilisticBroadcastHeader>();
    auto payloadLength = networkHeader->getPayloadLengthField();
    if (packet->getDataLength() < payloadLength) {
        throw cRuntimeError("Data error: illegal payload length"); // FIXME packet drop
    }
    if (packet->getDataLength() > payloadLength)
        packet->setBackOffset(packet->getFrontOffset() + payloadLength);
    auto payloadProtocol = networkHeader->getProtocol();
    packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&getProtocol());
    packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(networkHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<L3AddressInd>()->setSrcAddress(networkHeader->getSrcAddr());
}

/**
 * Attaches a "control info" structure (object) to the down message pMsg.
 */
void ProbabilisticBroadcast::setDownControlInfo(Packet *const pMsg, const MacAddress& pDestAddr)
{
    pMsg->addTagIfAbsent<MacAddressReq>()->setDestAddress(pDestAddr);
    pMsg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::probabilistic);
    pMsg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::probabilistic);
}

} // namespace inet

