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

#include <limits>
#include <algorithm>

#include "inet/networklayer/wiseroute/WiseRoute.h"
#include "inet/common/INETMath.h"
#include "inet/networklayer/common/IL3AddressType.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/common/FindModule.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/networklayer/common/SimpleNetworkProtocolControlInfo.h"

namespace inet {

using std::make_pair;

Define_Module(WiseRoute);

void WiseRoute::initialize(int stage)
{
    NetworkProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        arp = check_and_cast<IARP *>(getParentModule()->getSubmodule("arp"));
        headerLength = par("headerLength");
        rssiThreshold = par("rssiThreshold").doubleValue();
        rssiThreshold = math::dBm2mW(rssiThreshold);
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
    else if (stage == INITSTAGE_NETWORK_LAYER_3) {
        L3AddressResolver addressResolver;
        sinkAddress = addressResolver.resolve(par("sinkAddress"));

        IInterfaceTable *interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        myNetwAddr = interfaceTable->getInterface(1)->getNetworkAddress();

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
        WiseRouteDatagram *pkt = new WiseRouteDatagram("route-flood", ROUTE_FLOOD);
        L3Address broadcastAddress = myNetwAddr.getAddressType()->getBroadcastAddress();
        pkt->setByteLength(headerLength);
        pkt->setInitialSrcAddr(myNetwAddr);
        pkt->setFinalDestAddr(broadcastAddress);
        pkt->setSrcAddr(myNetwAddr);
        pkt->setDestAddr(broadcastAddress);
        pkt->setNbHops(0);
        floodTable.insert(make_pair(myNetwAddr, floodSeqNumber));
        pkt->setSeqNum(floodSeqNumber);
        floodSeqNumber++;
        pkt->setIsFlood(1);
        setDownControlInfo(pkt, MACAddress::BROADCAST_ADDRESS);
        sendDown(pkt);
        nbFloodsSent++;
        nbRouteFloodsSent++;
        scheduleAt(simTime() + routeFloodsInterval + uniform(0, 1), routeFloodTimer);
    }
    else {
        EV << "WiseRoute - handleSelfMessage: got unexpected message of kind " << msg->getKind() << endl;
        delete msg;
    }
}

void WiseRoute::handleLowerPacket(cPacket *msg)
{
    WiseRouteDatagram *netwMsg = check_and_cast<WiseRouteDatagram *>(msg);
    const L3Address& finalDestAddr = netwMsg->getFinalDestAddr();
    const L3Address& initialSrcAddr = netwMsg->getInitialSrcAddr();
    const L3Address& srcAddr = netwMsg->getSrcAddr();
    IMACProtocolControlInfo *ctrlInfo = check_and_cast<IMACProtocolControlInfo *>(netwMsg->getControlInfo());
    // KLUDGE: TODO: get rssi and ber
    EV_ERROR << "Getting RSSI and BER from the received frame is not yet implemented. Using default values.\n";
    double rssi = 1;    // TODO: ctrlInfo->getRSSI();
    double ber = 0;    // TODO: ctrlInfo->getBitErrorRate();
    // Check whether the message is a flood and if it has to be forwarded.
    floodTypes floodType = updateFloodTable(netwMsg->getIsFlood(), initialSrcAddr, finalDestAddr,
                netwMsg->getSeqNum());
    allReceivedRSSI.record(rssi);
    allReceivedBER.record(ber);
    if (floodType == DUPLICATE) {
        nbDuplicatedFloodsReceived++;
        delete netwMsg;
    }
    else {
        const cObject *pCtrlInfo = NULL;
        // If the message is a route flood, update the routing table.
        if (netwMsg->getKind() == ROUTE_FLOOD)
            updateRouteTable(initialSrcAddr, srcAddr, rssi, ber);

        if (finalDestAddr == myNetwAddr || finalDestAddr.isBroadcast()) {
            WiseRouteDatagram *msgCopy;
            if (floodType == FORWARD) {
                // it's a flood. copy for delivery, forward original.
                // if we are here (see updateFloodTable()), finalDestAddr == IP Broadcast. Hence finalDestAddr,
                // initialSrcAddr, and destAddr have already been correctly set
                // at origin, as well as the MAC control info. Hence only update
                // local hop source address.
                msgCopy = check_and_cast<WiseRouteDatagram *>(netwMsg->dup());
                netwMsg->setSrcAddr(myNetwAddr);
                pCtrlInfo = netwMsg->removeControlInfo();
                setDownControlInfo(netwMsg, MACAddress::BROADCAST_ADDRESS);
                netwMsg->setNbHops(netwMsg->getNbHops() + 1);
                sendDown(netwMsg);
                nbDataPacketsForwarded++;
            }
            else
                msgCopy = netwMsg;
            if (msgCopy->getKind() == DATA) {
                int protocol = msgCopy->getTransportProtocol();
                sendUp(decapsMsg(msgCopy), protocol);
                nbDataPacketsReceived++;
            }
            else {
                nbRouteFloodsReceived++;
                delete msgCopy;
            }
        }
        else {
            // not for me. if flood, forward as flood. else select a route
            if (floodType == FORWARD) {
                netwMsg->setSrcAddr(myNetwAddr);
                pCtrlInfo = netwMsg->removeControlInfo();
                setDownControlInfo(netwMsg, MACAddress::BROADCAST_ADDRESS);
                netwMsg->setNbHops(netwMsg->getNbHops() + 1);
                sendDown(netwMsg);
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
                netwMsg->setSrcAddr(myNetwAddr);
                netwMsg->setDestAddr(nextHop);
                pCtrlInfo = netwMsg->removeControlInfo();
                MACAddress nextHopMacAddr = arp->resolveL3Address(nextHop, NULL);    //FIXME interface entry pointer needed
                if (nextHopMacAddr.isUnspecified())
                    throw cRuntimeError("Cannot immediately resolve MAC address. Please configure a GenericARP module.");
                setDownControlInfo(netwMsg, nextHopMacAddr);
                netwMsg->setNbHops(netwMsg->getNbHops() + 1);
                sendDown(netwMsg);
                nbDataPacketsForwarded++;
                nbPureUnicastForwarded++;
            }
        }
        if (pCtrlInfo != NULL)
            delete pCtrlInfo;
    }
}

void WiseRoute::handleUpperPacket(cPacket *msg)
{
    L3Address finalDestAddr;
    L3Address nextHopAddr;
    MACAddress nextHopMacAddr;
    WiseRouteDatagram *pkt = new WiseRouteDatagram(msg->getName(), DATA);
    INetworkProtocolControlInfo *cInfo = check_and_cast<INetworkProtocolControlInfo *>(msg->removeControlInfo());

    pkt->setByteLength(headerLength);

    if (cInfo == NULL) {
        EV << "WiseRoute warning: Application layer did not specifiy a destination L3 address\n"
           << "\tusing broadcast address instead\n";
        finalDestAddr = myNetwAddr.getAddressType()->getBroadcastAddress();
    }
    else {
        EV << "WiseRoute: CInfo removed, netw addr=" << cInfo->getDestinationAddress() << endl;
        finalDestAddr = cInfo->getDestinationAddress();
    }

    pkt->setFinalDestAddr(finalDestAddr);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setNbHops(0);
    pkt->setTransportProtocol(cInfo->getTransportProtocol());
    delete cInfo;

    if (finalDestAddr.isBroadcast())
        nextHopAddr = myNetwAddr.getAddressType()->getBroadcastAddress();
    else
        nextHopAddr = getRoute(finalDestAddr, true);
    pkt->setDestAddr(nextHopAddr);
    if (nextHopAddr.isBroadcast()) {
        // it's a flood.
        nextHopMacAddr = MACAddress::BROADCAST_ADDRESS;
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
        nextHopMacAddr = arp->resolveL3Address(nextHopAddr, NULL);    //FIXME interface entry pointer needed
        if (nextHopMacAddr.isUnspecified())
            throw cRuntimeError("Cannot immediately resolve MAC address. Please configure a GenericARP module.");
    }
    setDownControlInfo(pkt, nextHopMacAddr);
    pkt->encapsulate(static_cast<cPacket *>(msg));
    sendDown(pkt);
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
    tRouteTable::iterator pos;

    pos = routeTable.find(origin);
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
//		tRouteTableEntry entry = pos->second;
//		if (entry.rssi > rssiThreshold) {
//			entry.nextHop = lastHop;
//			entry.rssi = rssi;
//			if (origin == 0)
//				nextHopSelectionForSink.record(lastHop);
//		}
    }
}

cMessage *WiseRoute::decapsMsg(WiseRouteDatagram *msg)
{
    cMessage *m = msg->decapsulate();
    SimpleNetworkProtocolControlInfo *const controlInfo = new SimpleNetworkProtocolControlInfo();
    controlInfo->setSourceAddress(msg->getInitialSrcAddr());
    controlInfo->setTransportProtocol(msg->getTransportProtocol());
    m->setControlInfo(controlInfo);
    nbHops = nbHops + msg->getNbHops();
    // delete the netw packet
    delete msg;
    return m;
}

WiseRoute::floodTypes WiseRoute::updateFloodTable(bool isFlood, const tFloodTable::key_type& srcAddr, const tFloodTable::key_type& destAddr, unsigned long seqNum)
{
    if (isFlood) {
        tFloodTable::iterator pos = floodTable.lower_bound(srcAddr);
        tFloodTable::iterator posEnd = floodTable.upper_bound(srcAddr);

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
cObject *WiseRoute::setDownControlInfo(cMessage *const pMsg, const MACAddress& pDestAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setDest(pDestAddr);
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}

} // namespace inet

