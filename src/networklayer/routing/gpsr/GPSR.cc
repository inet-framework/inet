//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "GPSR.h"
#include "InterfaceTableAccess.h"
#include "IPProtocolId_m.h"
#include "IPSocket.h"
#include "NodeOperations.h"

Define_Module(GPSR);

#define GPSR_EV EV << "GPSR at " << getHostName() << " "

// TODO: use some header?
static double const NaN = 0.0 / 0.0;

static inline double determinant(double a1, double a2, double b1, double b2) {
    return a1 * b2 - a2 * b1;
}

static inline bool isNaN(double d) { return d != d;}

// KLUDGE: implement position registry protocol
PositionTable GPSR::globalPositionTable;

GPSR::GPSR()
{
    host = NULL;
    nodeStatus = NULL;
    mobility = NULL;
    addressType = NULL;
    interfaceTable = NULL;
    routingTable = NULL;
    networkProtocol = NULL;
    beaconTimer = NULL;
    purgeNeighborsTimer = NULL;
}

GPSR::~GPSR()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(purgeNeighborsTimer);
}

//
// module interface
//

void GPSR::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        // GPSR parameters
        planarizationMode = (GPSRPlanarizationMode)(int)par("planarizationMode");
        interfaces = par("interfaces");
        beaconInterval = par("beaconInterval");
        maxJitter = par("maxJitter");
        neighborValidityInterval = par("neighborValidityInterval");
        // context
        host = getContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        interfaceTable = InterfaceTableAccess().get(this);
        mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
        routingTable = check_and_cast<IRoutingTable *>(getModuleByPath(par("routingTableModule")));
        networkProtocol = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolModule")));
        // internal
        beaconTimer = new cMessage("BeaconTimer");
        purgeNeighborsTimer = new cMessage("PurgeNeighborsTimer");
        scheduleBeaconTimer();
        schedulePurgeNeighborsTimer();
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS)
    {
        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_MANET);

        globalPositionTable.clear();
        host->subscribe(NF_LINK_BREAK, this);
        addressType = getSelfAddress().getAddressType();
        networkProtocol->registerHook(0, this);
        if (isNodeUp())
            configureInterfaces();
    }
}

void GPSR::handleMessage(cMessage * message)
{
    if (message->isSelfMessage())
        processSelfMessage(message);
    else
        processMessage(message);
}

//
// handling messages
//

void GPSR::processSelfMessage(cMessage * message)
{
    if (message == beaconTimer)
        processBeaconTimer();
    else if (message == purgeNeighborsTimer)
        processPurgeNeighborsTimer();
    else
        throw cRuntimeError("Unknown self message");
}

void GPSR::processMessage(cMessage * message)
{
    if (dynamic_cast<UDPPacket *>(message))
        processUDPPacket((UDPPacket *)message);
    else
        throw cRuntimeError("Unknown message");
}

//
// beacon timers
//

void GPSR::scheduleBeaconTimer()
{
    GPSR_EV << "Scheduling beacon timer" << endl;
    scheduleAt(simTime() + beaconInterval, beaconTimer);
}

void GPSR::processBeaconTimer()
{
    GPSR_EV << "Processing beacon timer" << endl;
    Address selfAddress = getSelfAddress();
    if (!selfAddress.isUnspecified()) {
        sendBeacon(createBeacon(), uniform(0, maxJitter).dbl());
        // KLUDGE: implement position registry protocol
        globalPositionTable.setPosition(selfAddress, mobility->getCurrentPosition());
    }
    scheduleBeaconTimer();
    schedulePurgeNeighborsTimer();
}

//
// handling purge neighbors timers
//

void GPSR::schedulePurgeNeighborsTimer()
{
    GPSR_EV << "Scheduling purge neighbors timer" << endl;
    simtime_t nextExpiration = getNextNeighborExpiration();
    if (nextExpiration == SimTime::getMaxTime()) {
        if (purgeNeighborsTimer->isScheduled())
            cancelEvent(purgeNeighborsTimer);
    }
    else {
        if (!purgeNeighborsTimer->isScheduled())
            scheduleAt(nextExpiration, purgeNeighborsTimer);
        else {
            if (purgeNeighborsTimer->getArrivalTime() != nextExpiration) {
                cancelEvent(purgeNeighborsTimer);
                scheduleAt(nextExpiration, purgeNeighborsTimer);
            }
        }
    }
}

void GPSR::processPurgeNeighborsTimer()
{
    GPSR_EV << "Processing purge neighbors timer" << endl;
    purgeNeighbors();
    schedulePurgeNeighborsTimer();
}

//
// handling UDP packets
//

void GPSR::sendUDPPacket(UDPPacket * packet, double delay)
{
    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

void GPSR::processUDPPacket(UDPPacket * packet)
{
    cPacket * encapsulatedPacket = packet->decapsulate();
    if (dynamic_cast<GPSRBeacon *>(encapsulatedPacket))
        processBeacon((GPSRBeacon *)encapsulatedPacket);
    else
        throw cRuntimeError("Unknown UDP packet");
    delete packet;
}

//
// handling beacons
//

GPSRBeacon * GPSR::createBeacon()
{
    GPSRBeacon * beacon = new GPSRBeacon();
    beacon->setAddress(getSelfAddress());
    beacon->setPosition(mobility->getCurrentPosition());
    return beacon;
}

void GPSR::sendBeacon(GPSRBeacon * beacon, double delay)
{
    GPSR_EV << "Sending beacon: address = " << beacon->getAddress() << ", position = " << beacon->getPosition() << endl;
    INetworkProtocolControlInfo * networkProtocolControlInfo = addressType->createNetworkProtocolControlInfo();
    networkProtocolControlInfo->setProtocol(IP_PROT_MANET);
    networkProtocolControlInfo->setHopLimit(255);
    networkProtocolControlInfo->setDestinationAddress(addressType->getLinkLocalManetRoutersMulticastAddress());
    networkProtocolControlInfo->setSourceAddress(getSelfAddress());
    UDPPacket * udpPacket = new UDPPacket(beacon->getName());
    udpPacket->encapsulate(beacon);
    udpPacket->setSourcePort(GPSR_UDP_PORT);
    udpPacket->setDestinationPort(GPSR_UDP_PORT);
    udpPacket->setControlInfo(dynamic_cast<cObject *>(networkProtocolControlInfo));
    sendUDPPacket(udpPacket, delay);
}

void GPSR::processBeacon(GPSRBeacon * beacon)
{
    GPSR_EV << "Processing beacon: address = " << beacon->getAddress() << ", position = " << beacon->getPosition() << endl;
    neighborPositionTable.setPosition(beacon->getAddress(), beacon->getPosition());
    delete beacon;
}

//
// handling packets
//

GPSRPacket * GPSR::createPacket(Address destination, cPacket * content)
{
    GPSRPacket * gpsrPacket = new GPSRPacket(content->getName());
    gpsrPacket->setRoutingMode(GPSR_GREEDY_ROUTING);
    // KLUDGE: implement position registry protocol
    gpsrPacket->setDestinationPosition(getDestinationPosition(destination));
    gpsrPacket->setBitLength(computePacketBitLength(gpsrPacket));
    gpsrPacket->encapsulate(content);
    return gpsrPacket;
}

int GPSR::computePacketBitLength(GPSRPacket * packet)
{
    // routingMode
    int routingMode = 1;
    // destinationPosition, perimeterRoutingStartPosition, perimeterRoutingForwardPosition
    int positions = 8 * 3 * 2 * 4;
    // currentFaceFirstSenderAddress, currentFaceFirstReceiverAddress, senderAddress
    int addresses = 8 * 3 * 4;
    // TODO: address size
    return routingMode + positions + addresses;
}

//
// configuration
//

bool GPSR::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void GPSR::configureInterfaces()
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry * interfaceEntry = interfaceTable->getInterface(i);
        if (interfaceEntry->isMulticast() && interfaceMatcher.matches(interfaceEntry->getName()))
            interfaceEntry->joinMulticastGroup(addressType->getLinkLocalManetRoutersMulticastAddress());
    }
}

//
// position
//

Coord GPSR::intersectSections(Coord & begin1, Coord & end1, Coord & begin2, Coord & end2)
{
    double x1 = begin1.x;
    double y1 = begin1.y;
    double x2 = end1.x;
    double y2 = end1.y;
    double x3 = begin2.x;
    double y3 = begin2.y;
    double x4 = end2.x;
    double y4 = end2.y;
    double a = determinant(x1, y1, x2, y2);
    double b = determinant(x3, y3, x4, y4);
    double c = determinant(x1 - x2, y1 - y2, x3 - x4, y3 - y4);
    double x = determinant(a, x1 - x2, b, x3 - x4) / c;
    double y = determinant(a, y1 - y2, b, y3 - y4) / c;
    if (x1 < x && x < x2 && x3 < x && x < x4 && y1 < y && y < y2 && y3 < y && y < y4)
        return Coord(x, y, 0);
    else
        return Coord(NaN, NaN, NaN);
}

Coord GPSR::getDestinationPosition(const Address & address)
{
    // KLUDGE: implement position registry protocol
    return globalPositionTable.getPosition(address);
}

Coord GPSR::getNeighborPosition(const Address & address)
{
    return neighborPositionTable.getPosition(address);
}

//
// angle
//

double GPSR::getVectorAngle(Coord vector)
{
    double angle = atan2(-vector.y, vector.x);
    if (angle < 0)
        angle += 2 * PI;
    return angle;
}

double GPSR::getDestinationAngle(const Address & address)
{
    return getVectorAngle(getDestinationPosition(address) - mobility->getCurrentPosition());
}

double GPSR::getNeighborAngle(const Address & address)
{
    return getVectorAngle(getNeighborPosition(address) - mobility->getCurrentPosition());
}

//
// address
//

std::string GPSR::getHostName()
{
    return host->getFullName();
}

Address GPSR::getSelfAddress()
{
    return routingTable->getRouterIdAsGeneric();
}

Address GPSR::getSenderNeighborAddress(INetworkDatagram * datagram)
{
    GPSRPacket * packet = check_and_cast<GPSRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    return packet->getSenderAddress();
}

//
// neighbor
//

simtime_t GPSR::getNextNeighborExpiration()
{
    simtime_t oldestPosition = neighborPositionTable.getOldestPosition();
    if (oldestPosition == SimTime::getMaxTime())
        return oldestPosition;
    else
        return oldestPosition + neighborValidityInterval;
}

void GPSR::purgeNeighbors()
{
    neighborPositionTable.removeOldPositions(simTime() - neighborValidityInterval);
}

std::vector<Address> GPSR::getPlanarNeighbors()
{
    std::vector<Address> planarNeighbors;
    std::vector<Address> neighborAddresses = neighborPositionTable.getAddresses();
    Coord selfPosition = mobility->getCurrentPosition();
    for (std::vector<Address>::iterator it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const Address & neighborAddress = *it;
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        if (planarizationMode == GPSR_RNG_PLANARIZATION) {
            double neighborDistance = (neighborPosition - selfPosition).length();
            for (std::vector<Address>::iterator jt = neighborAddresses.begin(); jt != neighborAddresses.end(); jt++) {
                const Address & witnessAddress = *jt;
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - selfPosition).length();;
                double neighborWitnessDistance = (witnessPosition - neighborPosition).length();
                if (*it == *jt)
                    continue;
                else if (neighborDistance > std::max(witnessDistance, neighborWitnessDistance))
                    goto eliminate;
            }
        }
        else if (planarizationMode == GPSR_GG_PLANARIZATION) {
            Coord middlePosition = (selfPosition + neighborPosition) / 2;
            double neighborDistance = (neighborPosition - middlePosition).length();
            for (std::vector<Address>::iterator jt = neighborAddresses.begin(); jt != neighborAddresses.end(); jt++) {
                const Address & witnessAddress = *jt;
                Coord witnessPosition = neighborPositionTable.getPosition(witnessAddress);
                double witnessDistance = (witnessPosition - middlePosition).length();;
                if (*it == *jt)
                    continue;
                else if (witnessDistance < neighborDistance)
                    goto eliminate;
            }
        }
        else
            throw cRuntimeError("Unknown planarization mode");
        planarNeighbors.push_back(*it);
        eliminate: ;
    }
    return planarNeighbors;
}

Address GPSR::getNextPlanarNeighborCounterClockwise(Address & startNeighborAddress, double startNeighborAngle)
{
    GPSR_EV << "Finding next planar neighbor (counter clockwise): startAddress = " << startNeighborAddress << ", startAngle = " << startNeighborAngle << endl;
    Address & bestNeighborAddress = startNeighborAddress;
    double bestNeighborAngleDifference = 2 * PI;
    std::vector<Address> neighborAddresses = getPlanarNeighbors();
    for (std::vector<Address>::iterator it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const Address & neighborAddress = *it;
        double neighborAngle = getNeighborAngle(neighborAddress);
        double neighborAngleDifference = neighborAngle - startNeighborAngle;
        if (neighborAngleDifference < 0)
            neighborAngleDifference += 2 * PI;
        GPSR_EV << "Trying next planar neighbor (counter clockwise): address = " << neighborAddress << ", angle = " << neighborAngle << endl;
        if (neighborAngleDifference != 0 && neighborAngleDifference < bestNeighborAngleDifference) {
            bestNeighborAngleDifference = neighborAngleDifference;
            bestNeighborAddress = neighborAddress;
        }
    }
    return bestNeighborAddress;
}

//
// next hop
//

Address GPSR::findNextHop(INetworkDatagram * datagram, const Address & destination)
{
    GPSRPacket * packet = check_and_cast<GPSRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    if (packet->getRoutingMode() == GPSR_GREEDY_ROUTING)
        return findGreedyRoutingNextHop(datagram, destination);
    else if (packet->getRoutingMode() == GPSR_PERIMETER_ROUTING)
        return findPerimeterRoutingNextHop(datagram, destination);
    else
        throw cRuntimeError("Unknown routing mode");
}

Address GPSR::findGreedyRoutingNextHop(INetworkDatagram * datagram, const Address & destination)
{
    GPSR_EV << "Finding next hop using greedy routing: destination = " << destination << endl;
    GPSRPacket * packet = check_and_cast<GPSRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord destinationPosition = packet->getDestinationPosition();
    double bestDistance = (destinationPosition - selfPosition).length();
    Address bestNeighbor;
    std::vector<Address> neighborAddresses = neighborPositionTable.getAddresses();
    for (std::vector<Address>::iterator it = neighborAddresses.begin(); it != neighborAddresses.end(); it++) {
        const Address & neighborAddress = *it;
        Coord neighborPosition = neighborPositionTable.getPosition(neighborAddress);
        double neighborDistance = (destinationPosition - neighborPosition).length();
        if (neighborDistance < bestDistance) {
            bestDistance = neighborDistance;
            bestNeighbor = neighborAddress;
        }
    }
    if (bestNeighbor.isUnspecified()) {
        GPSR_EV << "Switching to perimeter routing: destination = " << destination << endl;
        packet->setRoutingMode(GPSR_PERIMETER_ROUTING);
        packet->setPerimeterRoutingStartPosition(selfPosition);
        packet->setCurrentFaceFirstSenderAddress(selfAddress);
        packet->setCurrentFaceFirstReceiverAddress(Address());
        return findPerimeterRoutingNextHop(datagram, destination);
    }
    else
        return bestNeighbor;
}

Address GPSR::findPerimeterRoutingNextHop(INetworkDatagram * datagram, const Address & destination)
{
    GPSR_EV << "Finding next hop using perimeter routing: destination = " << destination << endl;
    GPSRPacket * packet = check_and_cast<GPSRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
    Address selfAddress = getSelfAddress();
    Coord selfPosition = mobility->getCurrentPosition();
    Coord perimeterRoutingStartPosition = packet->getPerimeterRoutingStartPosition();
    Coord destinationPosition = packet->getDestinationPosition();
    double selfDistance = (destinationPosition - selfPosition).length();
    double perimeterRoutingStartDistance = (destinationPosition - perimeterRoutingStartPosition).length();
    if (selfDistance < perimeterRoutingStartDistance) {
        GPSR_EV << "Switching to greedy routing: destination = " << destination << endl;
        packet->setRoutingMode(GPSR_GREEDY_ROUTING);
        packet->setPerimeterRoutingStartPosition(Coord());
        packet->setPerimeterRoutingForwardPosition(Coord());
        return findGreedyRoutingNextHop(datagram, destination);
    }
    else {
        Address & firstSenderAddress = packet->getCurrentFaceFirstSenderAddress();
        Address & firstReceiverAddress = packet->getCurrentFaceFirstReceiverAddress();
        Address nextNeighborAddress = getSenderNeighborAddress(datagram);
        bool hasIntersection;
        do {
            if (nextNeighborAddress.isUnspecified())
                nextNeighborAddress = getNextPlanarNeighborCounterClockwise(nextNeighborAddress, getDestinationAngle(destination));
            else
                nextNeighborAddress = getNextPlanarNeighborCounterClockwise(nextNeighborAddress, getNeighborAngle(nextNeighborAddress));
            if (nextNeighborAddress.isUnspecified())
                break;
            GPSR_EV << "Intersecting towards next hop: nextNeighbor = " << nextNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            Coord nextNeighborPosition = getNeighborPosition(nextNeighborAddress);
            Coord intersection = intersectSections(perimeterRoutingStartPosition, destinationPosition, selfPosition, nextNeighborPosition);
            hasIntersection = !isNaN(intersection.x);
            if (hasIntersection) {
                GPSR_EV << "Edge to next hop intersects: intersection = " << intersection << ", nextNeighbor = " << nextNeighborAddress << ", firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
                packet->setCurrentFaceFirstSenderAddress(selfAddress);
                packet->setCurrentFaceFirstReceiverAddress(Address());
            }
        }
        while (hasIntersection);
        if (firstSenderAddress == selfAddress && firstReceiverAddress == nextNeighborAddress) {
            GPSR_EV << "End of perimeter reached: firstSender = " << firstSenderAddress << ", firstReceiver = " << firstReceiverAddress << ", destination = " << destination << endl;
            return Address();
        }
        else {
            if (packet->getCurrentFaceFirstReceiverAddress().isUnspecified())
                packet->setCurrentFaceFirstReceiverAddress(nextNeighborAddress);
            return nextNeighborAddress;
        }
    }
}

//
// routing
//

INetfilter::IHook::Result GPSR::routeDatagram(INetworkDatagram * datagram, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop)
{
    const Address & source = datagram->getSourceAddress();
    const Address & destination = datagram->getDestinationAddress();
    GPSR_EV << "Finding next hop: source = " << source << ", destination = " << destination << endl;
    nextHop = findNextHop(datagram, destination);
    if (nextHop.isUnspecified()) {
        GPSR_EV << "No next hop found, dropping packet: source = " << source << ", destination = " << destination << endl;
        return DROP;
    }
    else {
        GPSR_EV << "Next hop found: source = " << source << ", destination = " << destination << ", nextHop: " << nextHop << endl;
        GPSRPacket * packet = check_and_cast<GPSRPacket *>(dynamic_cast<cPacket *>(datagram)->getEncapsulatedPacket());
        packet->setSenderAddress(getSelfAddress());
        // KLUDGE: find output interface
        outputInterfaceEntry = interfaceTable->getInterface(1);
        return ACCEPT;
    }
}

//
// netfilter
//

INetfilter::IHook::Result GPSR::datagramPreRoutingHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop)
{
    const Address & destination = datagram->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else
        return routeDatagram(datagram, outputInterfaceEntry, nextHop);
}

INetfilter::IHook::Result GPSR::datagramLocalInHook(INetworkDatagram * datagram, const InterfaceEntry * inputInterfaceEntry)
{
    cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
    GPSRPacket * gpsrPacket = dynamic_cast<GPSRPacket *>(networkPacket->getEncapsulatedPacket());
    if (gpsrPacket) {
        networkPacket->decapsulate();
        networkPacket->encapsulate(gpsrPacket->decapsulate());
        delete gpsrPacket;
    }
    return ACCEPT;
}

INetfilter::IHook::Result GPSR::datagramLocalOutHook(INetworkDatagram * datagram, const InterfaceEntry *& outputInterfaceEntry, Address & nextHop)
{
    const Address & destination = datagram->getDestinationAddress();
    if (destination.isMulticast() || destination.isBroadcast() || routingTable->isLocalAddress(destination))
        return ACCEPT;
    else {
        cPacket * networkPacket = dynamic_cast<cPacket *>(datagram);
        GPSRPacket * gpsrPacket = createPacket(datagram->getDestinationAddress(), networkPacket->decapsulate());
        networkPacket->encapsulate(gpsrPacket);
        return routeDatagram(datagram, outputInterfaceEntry, nextHop);
    }
}


//
// lifecycle
//

bool GPSR::handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation))  {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            configureInterfaces();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            // TODO: send a beacon to remove ourself from peers neighbor position table
            neighborPositionTable.clear();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            neighborPositionTable.clear();
    }
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

//
// notification
//

void GPSR::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method("receiveChangeNotification");
    if (signalID == NF_LINK_BREAK) {
        GPSR_EV << "Received link break" << endl;
        // TODO: shall we remove the neighbor?
    }
}
