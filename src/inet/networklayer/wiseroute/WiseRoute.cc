/***************************************************************************
 * file:        WiseRoute.cc
 *
 * author:      Damien Piguet, Jerome Rousselot
 *
 * copyright:   (C) 2007-2008 CSEM SA, Neuchatel, Switzerland.
 *
 * description: Implementation of the routing protocol of WiseStack.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * ported to Mixim 2.0.1 by Theodoros Kapourniotis
 * last modification: 06/02/11
 **************************************************************************/

#include <algorithm>
#include <limits>

#include "inet/common/FindModule.h"
#include "inet/common/INETMath.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/networklayer/wiseroute/WiseRoute.h"

namespace inet {

using std::make_pair;

Define_Module(WiseRoute);

void WiseRoute::initialize(int stage)
{
    NetworkProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        arp = getModuleFromPar<IArp>(par("arpModule"), this);
        headerLength = par("headerLength");
        rssiThreshold = math::dBmW2mW(par("rssiThreshold"));
        routeFloodsInterval = par("routeFloodsInterval");

        floodSeqNumber = 0;

        nbDataPacketsForwarded = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        nbDuplicatedFloodsReceived = 0;
        nbFloodsSent = 0;
        nbPureUnicastSent = 0;
        nbRouteFloodsSent = 0;
        nbRouteFloodsReceived = 0;
        nbUnicastFloodForwarded = 0;
        nbPureUnicastForwarded = 0;
        nbGetRouteFailures = 0;
        nbRoutesRecorded = 0;
        nbHops = 0;
        receivedRSSI.setName("receivedRSSI");
        routeRSSI.setName("routeRSSI");
        allReceivedRSSI.setName("allReceivedRSSI");
        receivedBER.setName("receivedBER");
        routeBER.setName("routeBER");
        allReceivedBER.setName("allReceivedBER");
        nextHopSelectionForSink.setName("nextHopSelectionForSink");

        routeFloodTimer = new cMessage("route-flood-timer", SEND_ROUTE_FLOOD_TIMER);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        L3AddressResolver addressResolver;
        sinkAddress = addressResolver.resolve(par("sinkAddress"));

        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        auto ie = interfaceTable->findFirstNonLoopbackInterface();
        if (ie != nullptr)
            myNetwAddr = ie->getNetworkAddress();
        else
            throw cRuntimeError("No non-loopback interface found!");

        // only schedule a flood of the node is a sink!!
        if (routeFloodsInterval > 0 && myNetwAddr == sinkAddress)
            scheduleAt(simTime() + uniform(0.5, 1.5), routeFloodTimer);
    }
}

WiseRoute::~WiseRoute()
{
    cancelAndDelete(routeFloodTimer);
}

void WiseRoute::handleSelfMessage(cMessage *msg)
{
    if (msg->getKind() == SEND_ROUTE_FLOOD_TIMER) {
        // Send route flood packet and restart the timer
        auto pkt = makeShared<WiseRouteHeader>();
        L3Address broadcastAddress = myNetwAddr.getAddressType()->getBroadcastAddress();
        pkt->setChunkLength(B(headerLength));
        pkt->setInitialSrcAddr(myNetwAddr);
        pkt->setFinalDestAddr(broadcastAddress);
        pkt->setSourceAddress(myNetwAddr);
        pkt->setDestinationAddress(broadcastAddress);
        pkt->setNbHops(0);
        floodTable.insert(make_pair(myNetwAddr, floodSeqNumber));
        pkt->setSeqNum(floodSeqNumber);
        floodSeqNumber++;
        pkt->setIsFlood(1);
        auto packet = new Packet("route-flood", ROUTE_FLOOD);
        packet->insertAtBack(pkt);
        setDownControlInfo(packet, MacAddress::BROADCAST_ADDRESS);
        sendDown(packet);
        nbFloodsSent++;
        nbRouteFloodsSent++;
        scheduleAt(simTime() + routeFloodsInterval + uniform(0, 1), routeFloodTimer);
    }
    else {
        EV << "WiseRoute - handleSelfMessage: got unexpected message of kind " << msg->getKind() << endl;
        delete msg;
    }
}

void WiseRoute::handleLowerPacket(Packet *packet)
{
    auto wiseRouteHeader = staticPtrCast<WiseRouteHeader>(packet->peekAtFront<WiseRouteHeader>()->dupShared());
    const L3Address& finalDestAddr = wiseRouteHeader->getFinalDestAddr();
    const L3Address& initialSrcAddr = wiseRouteHeader->getInitialSrcAddr();
    const L3Address& srcAddr = wiseRouteHeader->getSourceAddress();
    // KLUDGE: TODO: get rssi and ber
    EV_ERROR << "Getting RSSI and BER from the received frame is not yet implemented. Using default values.\n";
    double rssi = 1;    // TODO: ctrlInfo->getRSSI();
    double ber = 0;    // TODO: ctrlInfo->getBitErrorRate();
    // Check whether the message is a flood and if it has to be forwarded.
    floodTypes floodType = updateFloodTable(wiseRouteHeader->getIsFlood(), initialSrcAddr, finalDestAddr,
                wiseRouteHeader->getSeqNum());
    allReceivedRSSI.record(rssi);
    allReceivedBER.record(ber);
    if (floodType == DUPLICATE) {
        nbDuplicatedFloodsReceived++;
        delete packet;
    }
    else {
        const cObject *pCtrlInfo = nullptr;
        // If the message is a route flood, update the routing table.
        if (packet->getKind() == ROUTE_FLOOD)
            updateRouteTable(initialSrcAddr, srcAddr, rssi, ber);

        if (finalDestAddr == myNetwAddr || finalDestAddr.isBroadcast()) {
            Packet *packetCopy;
            if (floodType == FORWARD) {
                // it's a flood. copy for delivery, forward original.
                // if we are here (see updateFloodTable()), finalDestAddr == IP Broadcast. Hence finalDestAddr,
                // initialSrcAddr, and destAddr have already been correctly set
                // at origin, as well as the MAC control info. Hence only update
                // local hop source address.
                packetCopy = packet->dup();
                wiseRouteHeader->setSourceAddress(myNetwAddr);
                pCtrlInfo = packet->removeControlInfo();
                wiseRouteHeader->setNbHops(wiseRouteHeader->getNbHops() + 1);
                auto p = new Packet(packet->getName(), packet->getKind());
                packet->popAtFront<WiseRouteHeader>();
                p->insertAtBack(packet->peekDataAt(b(0), packet->getDataLength()));
                wiseRouteHeader->setPayloadLengthField(p->getDataLength());
                p->insertAtFront(wiseRouteHeader);
                setDownControlInfo(p, MacAddress::BROADCAST_ADDRESS);
                sendDown(p);
                nbDataPacketsForwarded++;
                delete packet;
            }
            else {
                packetCopy = packet;
            }
            if (packet->getKind() == DATA) {
                decapsulate(packetCopy);
                sendUp(packetCopy);
                nbDataPacketsReceived++;
            }
            else {
                nbRouteFloodsReceived++;
                delete packetCopy;
            }
        }
        else {
            // not for me. if flood, forward as flood. else select a route
            if (floodType == FORWARD) {
                wiseRouteHeader->setSourceAddress(myNetwAddr);
                pCtrlInfo = packet->removeControlInfo();
                wiseRouteHeader->setNbHops(wiseRouteHeader->getNbHops() + 1);
                auto p = new Packet(packet->getName(), packet->getKind());
                packet->popAtFront<WiseRouteHeader>();
                p->insertAtBack(packet->peekDataAt(b(0), packet->getDataLength()));
                wiseRouteHeader->setPayloadLengthField(p->getDataLength());
                p->insertAtFront(wiseRouteHeader);
                setDownControlInfo(p, MacAddress::BROADCAST_ADDRESS);
                sendDown(p);
                nbDataPacketsForwarded++;
                nbUnicastFloodForwarded++;
            }
            else {
                L3Address nextHop = getRoute(finalDestAddr);
                if (nextHop.isBroadcast()) {
                    // no route exist to destination, attempt to send to final destination
                    nextHop = finalDestAddr;
                    nbGetRouteFailures++;
                }
                wiseRouteHeader->setSourceAddress(myNetwAddr);
                wiseRouteHeader->setDestinationAddress(nextHop);
                pCtrlInfo = packet->removeControlInfo();
                MacAddress nextHopMacAddr = arp->resolveL3Address(nextHop, nullptr);    //FIXME interface entry pointer needed
                if (nextHopMacAddr.isUnspecified())
                    throw cRuntimeError("Cannot immediately resolve MAC address. Please configure a GlobalArp module.");
                wiseRouteHeader->setNbHops(wiseRouteHeader->getNbHops() + 1);
                auto p = new Packet(packet->getName(), packet->getKind());
                packet->popAtFront<WiseRouteHeader>();
                p->insertAtBack(packet->peekDataAt(b(0), packet->getDataLength()));
                wiseRouteHeader->setPayloadLengthField(p->getDataLength());
                p->insertAtFront(wiseRouteHeader);
                setDownControlInfo(p, nextHopMacAddr);
                sendDown(p);
                nbDataPacketsForwarded++;
                nbPureUnicastForwarded++;
            }
            delete packet;
        }
        if (pCtrlInfo != nullptr)
            delete pCtrlInfo;
    }
}

void WiseRoute::handleUpperPacket(Packet *packet)
{
    L3Address finalDestAddr;
    L3Address nextHopAddr;
    MacAddress nextHopMacAddr;
    auto pkt = makeShared<WiseRouteHeader>();

    pkt->setChunkLength(B(headerLength));

    auto addrTag = packet->findTag<L3AddressReq>();
    if (addrTag == nullptr) {
        EV << "WiseRoute warning: Application layer did not specifiy a destination L3 address\n"
           << "\tusing broadcast address instead\n";
        finalDestAddr = myNetwAddr.getAddressType()->getBroadcastAddress();
    }
    else {
        L3Address destAddr = addrTag->getDestAddress();
        EV << "WiseRoute: CInfo removed, netw addr=" << destAddr << endl;
        finalDestAddr = destAddr;
    }

    pkt->setFinalDestAddr(finalDestAddr);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setSourceAddress(myNetwAddr);
    pkt->setNbHops(0);
    pkt->setProtocolId(static_cast<IpProtocolId>(ProtocolGroup::ipprotocol.getProtocolNumber(packet->getTag<PacketProtocolTag>()->getProtocol())));

    if (finalDestAddr.isBroadcast())
        nextHopAddr = myNetwAddr.getAddressType()->getBroadcastAddress();
    else
        nextHopAddr = getRoute(finalDestAddr, true);
    pkt->setDestinationAddress(nextHopAddr);
    if (nextHopAddr.isBroadcast()) {
        // it's a flood.
        nextHopMacAddr = MacAddress::BROADCAST_ADDRESS;
        pkt->setIsFlood(1);
        nbFloodsSent++;
        // record flood in flood table
        floodTable.insert(make_pair(myNetwAddr, floodSeqNumber));
        pkt->setSeqNum(floodSeqNumber);
        floodSeqNumber++;
        nbGetRouteFailures++;
    }
    else {
        pkt->setIsFlood(0);
        nbPureUnicastSent++;
        nextHopMacAddr = arp->resolveL3Address(nextHopAddr, nullptr);    //FIXME interface entry pointer needed
        if (nextHopMacAddr.isUnspecified())
            throw cRuntimeError("Cannot immediately resolve MAC address. Please configure a GlobalArp module.");
    }
    pkt->setPayloadLengthField(packet->getDataLength());
    packet->insertAtFront(pkt);
    packet->setKind(DATA);
    setDownControlInfo(packet, nextHopMacAddr);
    sendDown(packet);
    nbDataPacketsSent++;
}

void WiseRoute::finish()
{
    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
    recordScalar("nbDataPacketsSent", nbDataPacketsSent);
    recordScalar("nbDuplicatedFloodsReceived", nbDuplicatedFloodsReceived);
    recordScalar("nbFloodsSent", nbFloodsSent);
    recordScalar("nbPureUnicastSent", nbPureUnicastSent);
    recordScalar("nbRouteFloodsSent", nbRouteFloodsSent);
    recordScalar("nbRouteFloodsReceived", nbRouteFloodsReceived);
    recordScalar("nbUnicastFloodForwarded", nbUnicastFloodForwarded);
    recordScalar("nbPureUnicastForwarded", nbPureUnicastForwarded);
    recordScalar("nbGetRouteFailures", nbGetRouteFailures);
    recordScalar("nbRoutesRecorded", nbRoutesRecorded);
    recordScalar("meanNbHops", (double)nbHops / (double)nbDataPacketsReceived);
}

void WiseRoute::updateRouteTable(const L3Address& origin, const L3Address& lastHop, double rssi, double ber)
{
    auto pos = routeTable.find(origin);
    receivedRSSI.record(rssi);
    receivedBER.record(ber);
    if (pos == routeTable.end()) {
        // A route towards origin does not exist yet. Insert the currently discovered one
        // only if the received RSSI is above the threshold.
        if (rssi > rssiThreshold) {
            tRouteTableEntry newEntry;

            // last hop from origin means next hop towards origin.
            newEntry.nextHop = lastHop;
            newEntry.rssi = rssi;
            routeRSSI.record(rssi);
            routeBER.record(ber);
            routeTable.insert(make_pair(origin, newEntry));
            nbRoutesRecorded++;
            if (origin.isUnspecified()) {
                // TODO: nextHopSelectionForSink.record(static_cast<double>(lastHop));
            }
        }
    }
    else {
        // A route towards the node which originated the received packet already exists.
        // Replace its entry only if the route proposal that we just received has a stronger
        // RSSI.
//        tRouteTableEntry entry = pos->second;
//        if (entry.rssi > rssiThreshold) {
//            entry.nextHop = lastHop;
//            entry.rssi = rssi;
//            if (origin == 0)
//                nextHopSelectionForSink.record(lastHop);
//        }
    }
}

void WiseRoute::decapsulate(Packet *packet)
{
    auto wiseRouteHeader = packet->popAtFront<WiseRouteHeader>();
    auto payloadLength = wiseRouteHeader->getPayloadLengthField();
    if (packet->getDataLength() < payloadLength) {
        throw cRuntimeError("Data error: illegal payload length");     //FIXME packet drop
    }
    if (packet->getDataLength() > payloadLength)
        packet->setBackOffset(packet->getFrontOffset() + payloadLength);
    auto payloadProtocol = wiseRouteHeader->getProtocol();
    packet->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&getProtocol());
    packet->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(wiseRouteHeader);
    packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(payloadProtocol);
    packet->addTagIfAbsent<L3AddressInd>()->setSrcAddress(wiseRouteHeader->getInitialSrcAddr());
    nbHops = nbHops + wiseRouteHeader->getNbHops();
}

WiseRoute::floodTypes WiseRoute::updateFloodTable(bool isFlood, const tFloodTable::key_type& srcAddr, const tFloodTable::key_type& destAddr, unsigned long seqNum)
{
    if (isFlood) {
        auto pos = floodTable.lower_bound(srcAddr);
        auto posEnd = floodTable.upper_bound(srcAddr);

        while (pos != posEnd) {
            if (seqNum == pos->second)
                return DUPLICATE; // this flood is known, don't forward it.
            ++pos;
        }
        floodTable.insert(make_pair(srcAddr, seqNum));
        if (destAddr == myNetwAddr)
            return FORME;
        else
            return FORWARD;
    }
    else
        return NOTAFLOOD;
}

WiseRoute::tFloodTable::key_type WiseRoute::getRoute(const tFloodTable::key_type& destAddr, bool    /*iAmOrigin*/) const
{
    // Find a route to dest address. As in the embedded code, if no route exists, indicate
    // final destination as next hop. If we'are lucky, final destination is one hop away...
    // If I am the origin of the packet and no route exists, use flood, hence return broadcast
    // address for next hop.
    tRouteTable::const_iterator pos = routeTable.find(destAddr);
    if (pos != routeTable.end())
        return pos->second.nextHop;
    else
        return myNetwAddr.getAddressType()->getBroadcastAddress();
}

/**
 * Attaches a "control info" structure (object) to the down message pMsg.
 */
void WiseRoute::setDownControlInfo(Packet *const pMsg, const MacAddress& pDestAddr)
{
    pMsg->addTagIfAbsent<MacAddressReq>()->setDestAddress(pDestAddr);
    pMsg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&getProtocol());
    pMsg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&getProtocol());
}

} // namespace inet

