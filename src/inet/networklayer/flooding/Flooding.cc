//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//
/* -*- mode:c++ -*- ********************************************************
 * file:        Flooding.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/

#include "inet/networklayer/flooding/Flooding.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"

namespace inet {

using std::endl;

Define_Module(Flooding);

/**
 * Reads all parameters from the ini file. If a parameter is not
 * specified in the ini file a default value will be set.
 **/
void Flooding::initialize(int stage)
{
    NetworkProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // initialize seqence number to 0
        seqNum = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        nbDataPacketsForwarded = 0;
        nbHops = 0;
        headerLength = par("headerLength");
        defaultTtl = par("defaultTtl");
        plainFlooding = par("plainFlooding");

        EV << "defaultTtl = " << defaultTtl
           << " plainFlooding = " << plainFlooding << endl;

        if (plainFlooding) {
            // these parameters are only needed for plain flooding
            bcMaxEntries = par("bcMaxEntries");
            bcDelTime = par("bcDelTime");
            EV << "bcMaxEntries = " << bcMaxEntries
               << " bcDelTime = " << bcDelTime << endl;
        }
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

void Flooding::finish()
{
    if (plainFlooding) {
        bcMsgs.clear();
    }
    recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
    recordScalar("nbDataPacketsSent", nbDataPacketsSent);
    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    if (nbDataPacketsReceived > 0) {
        recordScalar("meanNbHops", (double)nbHops / (double)nbDataPacketsReceived);
    }
    else {
        recordScalar("meanNbHops", 0);
    }
}

/**
 * All messages have to get a sequence number and the ttl filed has to
 * be specified. Afterwards the messages can be handed to the mac
 * layer. The mac address is set to -1 (broadcast address) because the
 * message is flooded (i.e. has to be send to all neighbors)
 *
 * In the case of plain flooding the message sequence number and
 * source address has also be stored in the bcMsgs list, so that this
 * message will not be rebroadcasted, if a copy will be flooded back
 * from the neigbouring nodes.
 *
 * If the maximum number of entries is reached the first (oldest) entry
 * is deleted.
 **/
void Flooding::handleUpperPacket(Packet *packet)
{
    encapsulate(packet);
    auto floodHeader = packet->peekAtFront<FloodingHeader>();

    if (plainFlooding) {
        if (bcMsgs.size() >= bcMaxEntries) {
            // serach the broadcast list of outdated entries and delete them
            for (auto it = bcMsgs.begin(); it != bcMsgs.end();) {
                if (it->delTime < simTime())
                    it = bcMsgs.erase(it);
                else
                    ++it;
            }
            // delete oldest entry if max size is reached
            if (bcMsgs.size() >= bcMaxEntries) {
                EV << "bcMsgs is full, delete oldest entry" << endl;
                bcMsgs.pop_front();
            }
        }
        bcMsgs.push_back(Bcast(floodHeader->getSeqNum(), floodHeader->getSourceAddress(), simTime() + bcDelTime));
    }
    // there is no routing so all messages are broadcast for the mac layer
    sendDown(packet);
    nbDataPacketsSent++;
}

/**
 * Messages from the mac layer will be forwarded to the application
 * only if the are broadcast or destined for this node.
 *
 * If the arrived message is a broadcast message it is also reflooded
 * only if the tll field is bigger than one. Before the message is
 * handed back to the mac layer the ttl field is reduced by one to
 * account for this hop.
 *
 * In the case of plain flooding the message will only be processed if
 * there is no corresponding entry in the bcMsgs
 * list (@ref
 * notBroadcasted). Otherwise the message will be deleted.
 **/
void Flooding::handleLowerPacket(Packet *packet)
{
    auto floodHeader = packet->peekAtFront<FloodingHeader>();

    // msg not broadcasted yet
    if (notBroadcasted(floodHeader.get())) {
        // msg is for me
        if (interfaceTable->isLocalAddress(floodHeader->getDestinationAddress())) {
            EV << " data msg for me! send to Upper" << endl;
            nbHops = nbHops + (defaultTtl + 1 - floodHeader->getTtl());
            decapsulate(packet);
            sendUp(packet);
            nbDataPacketsReceived++;
        }
        // broadcast message
        else if (floodHeader->getDestinationAddress().isBroadcast()) {
            // check ttl and rebroadcast
            if (floodHeader->getTtl() > 1) {
                EV << " data msg BROADCAST! ttl = " << floodHeader->getTtl()
                   << " > 1 -> rebroadcast msg & send to upper\n";
                auto dMsg = packet->dup();
                forwardPacket(dMsg);
            }
            else
                EV << " max hops reached (ttl = " << floodHeader->getTtl() << ") -> only send to upper\n";

            // message has to be forwarded to upper layer
            nbHops = nbHops + (defaultTtl + 1 - floodHeader->getTtl());
            decapsulate(packet);
            sendUp(packet);
            nbDataPacketsReceived++;
        }
        // not for me -> rebroadcast
        else {
            // check ttl and rebroadcast
            if (floodHeader->getTtl() > 1) {
                EV << " data msg not for me! ttl = " << floodHeader->getTtl()
                   << " > 1 -> forward\n";
                forwardPacket(packet);
            }
            else {
                // max hops reached -> delete
                EV << " max hops reached (ttl = " << floodHeader->getTtl() << ") -> delete msg\n";
                delete packet;
            }
        }
    }
    else {
        EV << " data msg already BROADCASTed! delete msg\n";
        delete packet;
    }
}

void Flooding::forwardPacket(Packet *packet)
{
    auto floodingHeader = packet->removeAtFront<FloodingHeader>();
    floodingHeader->setTtl(floodingHeader->getTtl() - 1);
    packet->insertAtFront(floodingHeader);
    setDownControlInfo(packet, MacAddress::BROADCAST_ADDRESS);
    sendDown(packet);
    nbDataPacketsForwarded++;
}

/**
 * The bcMsgs list is searched for the arrived message. If the message
 * is in the list, it was already broadcasted and the function returns
 * false.
 *
 * Concurrently all outdated (older than bcDelTime) are deleted. If
 * the list is full and a new message has to be entered, the oldest
 * entry is deleted.
 **/
bool Flooding::notBroadcasted(const FloodingHeader *msg)
{
    if (!plainFlooding)
        return true;

    // serach the broadcast list of outdated entries and delete them
    for (auto it = bcMsgs.begin(); it != bcMsgs.end();) {
        if (it->delTime < simTime()) {
            it = bcMsgs.erase(it);
        }
        // message was already broadcasted
        else if ((it->srcAddr == msg->getSourceAddress()) && (it->seqNum == msg->getSeqNum())) {
            // update entry
            it->delTime = simTime() + bcDelTime;
            return false;
        }
        else
            ++it;
    }

    // delete oldest entry if max size is reached
    if (bcMsgs.size() >= bcMaxEntries) {
        EV << "bcMsgs is full, delete oldest entry\n";
        bcMsgs.pop_front();
    }

    bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getSourceAddress(), simTime() + bcDelTime));
    return true;
}

/**
 * Decapsulates the packet from the received Network packet
 **/
void Flooding::decapsulate(Packet *packet)
{
    auto floodHeader = packet->popAtFront<FloodingHeader>();
    auto payloadLength = floodHeader->getPayloadLengthField();
    if (packet->getDataLength() < payloadLength) {
        throw cRuntimeError("Data error: illegal payload length"); // FIXME packet drop
    }
    if (packet->getDataLength() > payloadLength)
        packet->setBackOffset(packet->getFrontOffset() + payloadLength);
    auto payloadProtocol = floodHeader->getProtocol();
    packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&getProtocol());
    packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(floodHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    auto addressInd = packet->addTagIfAbsent<L3AddressInd>();
    addressInd->setSrcAddress(floodHeader->getSourceAddress());
    addressInd->setDestAddress(floodHeader->getDestinationAddress());
    packet->addTagIfAbsent<HopLimitInd>()->setHopLimit(floodHeader->getTtl());
}

/**
 * Encapsulates the received ApplPkt into a NetwPkt and set all needed
 * header fields.
 **/
void Flooding::encapsulate(Packet *appPkt)
{
    L3Address netwAddr;

    EV << "in encaps...\n";

    auto cInfo = appPkt->removeControlInfo();
    auto pkt = makeShared<FloodingHeader>(); // TODO appPkt->getName(), appPkt->getKind());
    pkt->setChunkLength(b(headerLength));

    auto& hopLimitReq = appPkt->removeTagIfPresent<HopLimitReq>();
    int ttl = (hopLimitReq != nullptr) ? hopLimitReq->getHopLimit() : -1;
    if (ttl == -1)
        ttl = defaultTtl;

    pkt->setSeqNum(seqNum);
    seqNum++;
    pkt->setTtl(ttl);

    const auto& addressReq = appPkt->findTag<L3AddressReq>();
    if (addressReq == nullptr) {
        EV << "warning: Application layer did not specifiy a destination L3 address\n"
           << "\tusing broadcast address instead\n";
        netwAddr = netwAddr.getAddressType()->getBroadcastAddress();
    }
    else {
        pkt->setProtocol(appPkt->getTag<PacketProtocolTag>()->getProtocol());
        netwAddr = addressReq->getDestAddress();
        EV << "CInfo removed, netw addr=" << netwAddr << endl;
        delete cInfo;
    }

    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    EV << " netw " << myNetwAddr << " sending packet" << endl;

    EV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
       << " -> set destMac=L2BROADCAST" << endl;

    pkt->setPayloadLengthField(appPkt->getDataLength());

    // encapsulate the application packet
    setDownControlInfo(appPkt, MacAddress::BROADCAST_ADDRESS);

    appPkt->insertAtFront(pkt);
    EV << " pkt encapsulated\n";
}

/**
 * Attaches a "control info" structure (object) to the down message pMsg.
 */
void Flooding::setDownControlInfo(Packet *const pMsg, const MacAddress& pDestAddr)
{
    pMsg->addTagIfAbsent<MacAddressReq>()->setDestAddress(pDestAddr);
    pMsg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getProtocol());
    pMsg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&getProtocol());
}

} // namespace inet

