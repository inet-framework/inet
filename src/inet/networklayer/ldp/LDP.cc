//
// (C) 2005 Vojtech Janota
// (C) 2004 Andras Varga
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include <iostream>
#include <fstream>
#include <algorithm>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ldp/LDP.h"

//#include "inet/networklayer/mpls/ConstType.h"
#include "inet/networklayer/mpls/LIBTable.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/NotifierConsts.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/transportlayer/tcp_common/TCPSegment.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ted/TED.h"

namespace inet {

Define_Module(LDP);

std::ostream& operator<<(std::ostream& os, const LDP::fec_bind_t& f)
{
    os << "fecid=" << f.fecid << "  peer=" << f.peer << " label=" << f.label;
    return os;
}

bool fecPrefixCompare(const LDP::fec_t& a, const LDP::fec_t& b)
{
    return a.length > b.length;
}

std::ostream& operator<<(std::ostream& os, const LDP::fec_t& f)
{
    os << "fecid=" << f.fecid << "  addr=" << f.addr << "  length=" << f.length << "  nextHop=" << f.nextHop;
    return os;
}

std::ostream& operator<<(std::ostream& os, const LDP::pending_req_t& r)
{
    os << "fecid=" << r.fecid << "  peer=" << r.peer;
    return os;
}

std::ostream& operator<<(std::ostream& os, const LDP::peer_info& p)
{
    os << "peerIP=" << p.peerIP << "  interface=" << p.linkInterface
       << "  activeRole=" << (p.activeRole ? "true" : "false")
       << "  socket=" << (p.socket ? TCPSocket::stateName(p.socket->getState()) : "nullptr");
    return os;
}

bool operator==(const FEC_TLV& a, const FEC_TLV& b)
{
    return a.length == b.length && a.addr == b.addr;
}

bool operator!=(const FEC_TLV& a, const FEC_TLV& b)
{
    return !operator==(a, b);
}

std::ostream& operator<<(std::ostream& os, const FEC_TLV& a)
{
    os << "addr=" << a.addr << "  length=" << a.length;
    return os;
}

LDP::LDP()
{
}

LDP::~LDP()
{
    for (auto & elem : myPeers)
        cancelAndDelete(elem.timeout);

    cancelAndDelete(sendHelloMsg);
    //this causes segfault at the end of simulation       -- Vojta
    //socketMap.deleteSockets();
}

void LDP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    //FIXME move bind() and listen() calls to a new startModule() function, and call it from initialize() and from handleOperationStage()
    //FIXME register to InterfaceEntry changes, for detecting the interface add/delete, and detecting multicast config changes:
    //      should be refresh the udpSockets vector when interface added/deleted, or isMulticast() value changed.

    if (stage == INITSTAGE_LOCAL) {
        holdTime = par("holdTime").doubleValue();
        helloInterval = par("helloInterval").doubleValue();

        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        lt = getModuleFromPar<LIBTable>(par("libTableModule"), this);
        tedmod = getModuleFromPar<TED>(par("tedModule"), this);

        WATCH_VECTOR(myPeers);
        WATCH_VECTOR(fecUp);
        WATCH_VECTOR(fecDown);
        WATCH_VECTOR(fecList);
        WATCH_VECTOR(pending);

        maxFecid = 0;
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        // schedule first hello
        sendHelloMsg = new cMessage("LDPSendHello");
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        if (isNodeUp())
            scheduleAt(simTime() + exponential(0.1), sendHelloMsg);

        // bind UDP socket
        udpSocket.setOutputGate(gate("udpOut"));
        udpSocket.bind(LDP_PORT);
        for (int i = 0; i < ift->getNumInterfaces(); ++i) {
            InterfaceEntry *ie = ift->getInterface(i);
            if (ie->isMulticast()) {
                udpSockets.push_back(UDPSocket());
                udpSockets.back().setOutputGate(gate("udpOut"));
                udpSockets.back().setMulticastLoop(false);
                udpSockets.back().setMulticastOutputInterface(ie->getInterfaceId());
            }
        }

        // start listening for incoming TCP conns
        EV_INFO << "Starting to listen on port " << LDP_PORT << " for incoming LDP sessions\n";
        serverSocket.setOutputGate(gate("tcpOut"));
        serverSocket.readDataTransferModePar(*this);
        serverSocket.bind(LDP_PORT);
        serverSocket.listen();

        // build list of recognized FECs
        rebuildFecList();

        // listen for routing table modifications
        cModule *host = getContainingNode(this);
        host->subscribe(NF_ROUTE_ADDED, this);
        host->subscribe(NF_ROUTE_DELETED, this);
    }
}

void LDP::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("LDP is not running");
    EV_INFO << "Received: (" << msg->getClassName() << ")" << msg->getName() << "\n";
    if (msg == sendHelloMsg) {
        // every LDP capable router periodically sends HELLO messages to the
        // "all routers in the sub-network" multicast address
        EV_INFO << "Multicasting LDP Hello to neighboring routers\n";
        sendHelloTo(IPv4Address::ALL_ROUTERS_MCAST);

        // schedule next hello
        scheduleAt(simTime() + helloInterval, sendHelloMsg);
    }
    else if (msg->isSelfMessage()) {
        EV_INFO << "Timer " << msg->getName() << " expired\n";
        if (!strcmp(msg->getName(), "HelloTimeout")) {
            processHelloTimeout(msg);
        }
        else {
            processNOTIFICATION(check_and_cast<LDPNotify *>(msg));
        }
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "udpIn")) {
        // we can only receive LDP Hello from UDP (everything else goes over TCP)
        processLDPHello(check_and_cast<LDPHello *>(msg));
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "tcpIn")) {
        processMessageFromTCP(msg);
    }
}

bool LDP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            scheduleAt(simTime() + exponential(0.1), sendHelloMsg);
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER) {
            for (auto & elem : myPeers)
                cancelAndDelete(elem.timeout);
            myPeers.clear();
            cancelEvent(sendHelloMsg);
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
        }
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

bool LDP::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void LDP::sendToPeer(IPv4Address dest, cMessage *msg)
{
    getPeerSocket(dest)->send(msg);
}

void LDP::sendMappingRequest(IPv4Address dest, IPv4Address addr, int length)
{
    LDPLabelRequest *requestMsg = new LDPLabelRequest("Lb-Req");
    requestMsg->setByteLength(LDP_HEADER_BYTES);    // FIXME find out actual length
    requestMsg->setType(LABEL_REQUEST);

    FEC_TLV fec;
    fec.addr = addr;
    fec.length = length;
    requestMsg->setFec(fec);

    requestMsg->setReceiverAddress(dest);
    requestMsg->setSenderAddress(rt->getRouterId());

    sendToPeer(dest, requestMsg);
}

void LDP::updateFecListEntry(LDP::fec_t oldItem)
{
    // do we have mapping from downstream?
    auto dit = findFecEntry(fecDown, oldItem.fecid, oldItem.nextHop);

    // is next hop our LDP peer?
    bool ER = findPeerSocket(oldItem.nextHop) == nullptr;

    ASSERT(!(ER && dit != fecDown.end()));    // can't be egress and have mapping at the same time

    // adjust upstream mappings
    for (auto uit = fecUp.begin(); uit != fecUp.end(); ) {
        if (uit->fecid != oldItem.fecid) {
            uit++;
            continue;
        }

        std::string inInterface = findInterfaceFromPeerAddr(uit->peer);
        std::string outInterface = findInterfaceFromPeerAddr(oldItem.nextHop);
        if (ER) {
            // we are egress, that's easy:
            LabelOpVector outLabel = LIBTable::popLabel();
            uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);

            EV_DETAIL << "installed (egress) LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                      << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
            uit++;
        }
        else if (dit != fecDown.end()) {
            // we have mapping from DS, that's easy
            LabelOpVector outLabel = LIBTable::swapLabel(dit->label);
            uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);

            EV_DETAIL << "installed LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                      << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
            uit++;
        }
        else {
            // no mapping from DS, withdraw mapping US
            EV_INFO << "sending withdraw message upstream" << endl;
            sendMapping(LABEL_WITHDRAW, uit->peer, uit->label, oldItem.addr, oldItem.length);

            // remove from US mappings
            uit = fecUp.erase(uit);
        }
    }

    if (!ER && dit == fecDown.end()) {
        // and ask DS for mapping
        EV_INFO << "sending request message downstream" << endl;
        sendMappingRequest(oldItem.nextHop, oldItem.addr, oldItem.length);
    }
}

void LDP::rebuildFecList()
{
    EV_INFO << "make list of recognized FECs" << endl;

    FecVector oldList = fecList;
    fecList.clear();

    for (int i = 0; i < rt->getNumRoutes(); i++) {
        // every entry in the routing table

        const IPv4Route *re = rt->getRoute(i);

        // ignore multicast routes
        if (re->getDestination().isMulticast())
            continue;

        // find out current next hop according to routing table
        IPv4Address nextHop = (re->getGateway().isUnspecified()) ? re->getDestination() : re->getGateway();
        ASSERT(!nextHop.isUnspecified());

        EV_INFO << "nextHop <-- " << nextHop << endl;

        auto it = findFecEntry(oldList, re->getDestination(), re->getNetmask().getNetmaskLength());

        if (it == oldList.end()) {
            // fec didn't exist, it was just created
            fec_t newItem;
            newItem.fecid = ++maxFecid;
            newItem.addr = re->getDestination();
            newItem.length = re->getNetmask().getNetmaskLength();
            newItem.nextHop = nextHop;
            updateFecListEntry(newItem);
            fecList.push_back(newItem);
        }
        else if (it->nextHop != nextHop) {
            // next hop for this FEC changed,
            it->nextHop = nextHop;
            updateFecListEntry(*it);
            fecList.push_back(*it);
            oldList.erase(it);
        }
        else {
            // FEC didn't change, reusing old values
            fecList.push_back(*it);
            oldList.erase(it);
            continue;
        }
    }

    // our own addresses (XXX is it needed?)

    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->getNetworkLayerGateIndex() < 0)
            continue;
        if (!ie->ipv4Data())
            continue;

        auto it = findFecEntry(oldList, ie->ipv4Data()->getIPAddress(), 32);
        if (it == oldList.end()) {
            fec_t newItem;
            newItem.fecid = ++maxFecid;
            newItem.addr = ie->ipv4Data()->getIPAddress();
            newItem.length = 32;
            newItem.nextHop = ie->ipv4Data()->getIPAddress();
            fecList.push_back(newItem);
        }
        else {
            fecList.push_back(*it);
            oldList.erase(it);
        }
    }

    if (oldList.size() > 0) {
        EV_INFO << "there are " << oldList.size() << " deprecated FECs, removing them" << endl;

        for (auto & elem : oldList) {
            EV_DETAIL << "removing FEC= " << elem << endl;

            for (auto & _dit : fecDown) {
                if (_dit.fecid != elem.fecid)
                    continue;

                EV_DETAIL << "sending release label=" << _dit.label << " downstream to " << _dit.peer << endl;

                sendMapping(LABEL_RELEASE, _dit.peer, _dit.label, elem.addr, elem.length);
            }

            for (auto & _uit : fecUp) {
                if (_uit.fecid != elem.fecid)
                    continue;

                EV_DETAIL << "sending withdraw label=" << _uit.label << " upstream to " << _uit.peer << endl;

                sendMapping(LABEL_WITHDRAW, _uit.peer, _uit.label, elem.addr, elem.length);

                EV_DETAIL << "removing entry inLabel=" << _uit.label << " from LIB" << endl;

                lt->removeLibEntry(_uit.label);
            }
        }
    }

    // we must keep this list sorted for matching to work correctly
    // this is probably slower than it must be
    std::stable_sort(fecList.begin(), fecList.end(), fecPrefixCompare);
}

void LDP::updateFecList(IPv4Address nextHop)
{
    for (auto & elem : fecList) {
        if (elem.nextHop != nextHop)
            continue;

        updateFecListEntry(elem);
    }
}

void LDP::sendHelloTo(IPv4Address dest)
{
    LDPHello *hello = new LDPHello("LDP-Hello");
    hello->setByteLength(LDP_HEADER_BYTES);
    hello->setType(HELLO);
    hello->setSenderAddress(rt->getRouterId());
    //hello->setReceiverAddress(...);
    hello->setHoldTime(SIMTIME_DBL(holdTime));
    //hello->setRbit(...);
    //hello->setTbit(...);
    hello->addPar("color") = LDP_HELLO_TRAFFIC;

    if (dest.isMulticast()) {
        for (int i = 0; i < (int)udpSockets.size(); ++i) {
            LDPHello *msg = i == (int)udpSockets.size() - 1 ? hello : hello->dup();
            udpSockets[i].sendTo(msg, dest, LDP_PORT);
        }
    }
    else
        udpSocket.sendTo(hello, dest, LDP_PORT);
}

void LDP::processHelloTimeout(cMessage *msg)
{
    // peer is gone

    unsigned int i;
    for (i = 0; i < myPeers.size(); i++)
        if (myPeers[i].timeout == msg)
            break;

    ASSERT(i < myPeers.size());

    IPv4Address peerIP = myPeers[i].peerIP;

    EV_INFO << "peer=" << peerIP << " is gone, removing adjacency" << endl;

    ASSERT(!myPeers[i].timeout->isScheduled());
    delete myPeers[i].timeout;
    ASSERT(myPeers[i].socket);
    myPeers[i].socket->abort();    // should we only close?
    delete myPeers[i].socket;
    myPeers.erase(myPeers.begin() + i);

    EV_INFO << "removing (stale) bindings from fecDown for peer=" << peerIP << endl;

    for (auto dit = fecDown.begin(); dit != fecDown.end(); ) {
        if (dit->peer != peerIP) {
            dit++;
            continue;
        }

        EV_DETAIL << "label=" << dit->label << endl;

        // send release message just in case (?)
        // what happens if peer is not really down and
        // hello messages just disappeared?
        // does the protocol recover on its own (XXX check this)

        dit = fecDown.erase(dit);
    }

    EV_INFO << "removing bindings from sent to peer=" << peerIP << " from fecUp" << endl;

    for (auto uit = fecUp.begin(); uit != fecUp.end(); ) {
        if (uit->peer != peerIP) {
            uit++;
            continue;
        }

        EV_DETAIL << "label=" << uit->label << endl;

        // send withdraw message just in case (?)
        // see comment above...

        uit = fecUp.erase(uit);
    }

    EV_INFO << "updating fecList" << endl;

    updateFecList(peerIP);

    // update TED and routing table

    unsigned int index = tedmod->linkIndex(rt->getRouterId(), peerIP);
    tedmod->ted[index].state = false;
    announceLinkChange(index);
    tedmod->rebuildRoutingTable();
}

void LDP::processLDPHello(LDPHello *msg)
{
    UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
    //IPv4Address peerAddr = controlInfo->getSrcAddr().toIPv4();
    IPv4Address peerAddr = msg->getSenderAddress();
    int interfaceId = controlInfo->getInterfaceId();
    delete msg;

    EV_INFO << "Received LDP Hello from " << peerAddr << ", ";

    if (peerAddr.isUnspecified() || peerAddr == rt->getRouterId()) {
        // must be ourselves (we're also in the all-routers multicast group), ignore
        EV_INFO << "that's myself, ignore\n";
        return;
    }

    // mark link as working if it was failed, and rebuild table
    unsigned int index = tedmod->linkIndex(rt->getRouterId(), peerAddr);
    if (!tedmod->ted[index].state) {
        tedmod->ted[index].state = true;
        tedmod->rebuildRoutingTable();
        announceLinkChange(index);
    }

    // peer already in table?
    int i = findPeer(peerAddr);
    if (i != -1) {
        EV_DETAIL << "already in my peer table, rescheduling timeout" << endl;
        ASSERT(myPeers[i].timeout);
        cancelEvent(myPeers[i].timeout);
        scheduleAt(simTime() + holdTime, myPeers[i].timeout);
        return;
    }

    // not in table, add it
    peer_info info;
    info.peerIP = peerAddr;
    info.linkInterface = ift->getInterfaceById(interfaceId)->getName();
    info.activeRole = peerAddr.getInt() > rt->getRouterId().getInt();
    info.socket = nullptr;
    info.timeout = new cMessage("HelloTimeout");
    scheduleAt(simTime() + holdTime, info.timeout);
    myPeers.push_back(info);
    int peerIndex = myPeers.size() - 1;

    EV_INFO << "added to peer table\n";
    EV_INFO << "We'll be " << (info.activeRole ? "ACTIVE" : "PASSIVE") << " in this session\n";

    // introduce ourselves with a Hello, then connect if we're in ACTIVE role
    sendHelloTo(peerAddr);
    if (info.activeRole) {
        EV_INFO << "Establishing session with it\n";
        openTCPConnectionToPeer(peerIndex);
    }
}

void LDP::openTCPConnectionToPeer(int peerIndex)
{
    TCPSocket *socket = new TCPSocket();
    socket->setOutputGate(gate("tcpOut"));
    socket->setCallbackObject(this, (void *)((intptr_t)peerIndex));
    socket->readDataTransferModePar(*this);
    socket->bind(rt->getRouterId(), 0);
    socketMap.addSocket(socket);
    myPeers[peerIndex].socket = socket;

    socket->connect(myPeers[peerIndex].peerIP, LDP_PORT);
}

void LDP::processMessageFromTCP(cMessage *msg)
{
    TCPSocket *socket = socketMap.findSocketFor(msg);
    if (!socket) {
        // not yet in socketMap, must be new incoming connection.
        // find which peer it is and register connection
        socket = new TCPSocket(msg);
        socket->setOutputGate(gate("tcpOut"));
        socket->readDataTransferModePar(*this);

        // FIXME there seems to be some confusion here. Is it sure that
        // routerIds we use as peerAddrs are the same as IP addresses
        // the routing is based on? --Andras
        IPv4Address peerAddr = socket->getRemoteAddress().toIPv4();

        int i = findPeer(peerAddr);
        if (i == -1 || myPeers[i].socket) {
            // nothing known about this guy, or already connected: refuse
            socket->close();    // reset()?
            delete socket;
            delete msg;
            return;
        }
        myPeers[i].socket = socket;
        socket->setCallbackObject(this, (void *)((intptr_t)i));
        socketMap.addSocket(socket);
    }

    // dispatch to socketEstablished(), socketDataArrived(), socketPeerClosed()
    // or socketFailure()
    socket->processMessage(msg);
}

void LDP::socketEstablished(int, void *yourPtr)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV_INFO << "TCP connection established with peer " << peer.peerIP << "\n";

    // we must update all entries with nextHop == peerIP
    updateFecList(peer.peerIP);

    // FIXME start LDP session setup (if we're on the active side?)
}

void LDP::socketDataArrived(int, void *yourPtr, cPacket *msg, bool)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV_INFO << "Message arrived over TCP from peer " << peer.peerIP << "\n";

    delete msg->removeControlInfo();
    processLDPPacketFromTCP(check_and_cast<LDPPacket *>(msg));
}

void LDP::socketPeerClosed(int, void *yourPtr)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV_INFO << "Peer " << peer.peerIP << " closed TCP connection\n";

    ASSERT(false);

/*
    // close the connection (if not already closed)
    if (socket.getState()==TCPSocket::PEER_CLOSED)
    {
        EV << "remote TCP closed, closing here as well\n";
        close();
    }
 */
}

void LDP::socketClosed(int, void *yourPtr)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV_INFO << "TCP connection to peer " << peer.peerIP << " closed\n";

    ASSERT(false);

    // FIXME what now? reconnect after a delay?
}

void LDP::socketFailure(int, void *yourPtr, int code)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV_INFO << "TCP connection to peer " << peer.peerIP << " broken\n";

    ASSERT(false);

    // FIXME what now? reconnect after a delay?
}

void LDP::processLDPPacketFromTCP(LDPPacket *ldpPacket)
{
    switch (ldpPacket->getType()) {
        case HELLO:
            throw cRuntimeError("Received LDP HELLO over TCP (should arrive over UDP)");
            break;

        case ADDRESS:
            // processADDRESS(ldpPacket);
            throw cRuntimeError("Received LDP ADDRESS message, unsupported in this version");
            break;

        case ADDRESS_WITHDRAW:
            // processADDRESS_WITHDRAW(ldpPacket);
            throw cRuntimeError("LDP PROC DEBUG: Received LDP ADDRESS_WITHDRAW message, unsupported in this version");
            break;

        case LABEL_MAPPING:
            processLABEL_MAPPING(check_and_cast<LDPLabelMapping *>(ldpPacket));
            break;

        case LABEL_REQUEST:
            processLABEL_REQUEST(check_and_cast<LDPLabelRequest *>(ldpPacket));
            break;

        case LABEL_WITHDRAW:
            processLABEL_WITHDRAW(check_and_cast<LDPLabelMapping *>(ldpPacket));
            break;

        case LABEL_RELEASE:
            processLABEL_RELEASE(check_and_cast<LDPLabelMapping *>(ldpPacket));
            break;

        case NOTIFICATION:
            processNOTIFICATION(check_and_cast<LDPNotify *>(ldpPacket));
            break;

        default:
            throw cRuntimeError("LDP PROC DEBUG: Unrecognized LDP Message Type, type is %d", ldpPacket->getType());
            break;
    }
}

IPv4Address LDP::locateNextHop(IPv4Address dest)
{
    // Mapping L3 IP-host of next hop to L2 peer address.

    // Lookup the routing table, rfc3036
    // "When the FEC for which a label is requested is a Prefix FEC Element or
    //  a Host Address FEC Element, the receiving LSR uses its routing table to determine
    //  its response. Unless its routing table includes an entry that exactly matches
    //  the requested Prefix or Host Address, the LSR must respond with a
    //  No Route Notification message."
    //
    // FIXME the code below (though seems like that's what the RFC refers to) doesn't work
    // -- we can't reasonably expect the destination host to be exaplicitly in an
    // LSR's routing table!!! Use simple IP routing instead. --Andras
    //
    // Wrong code:
    //int i;
    //for (i=0; i < rt->getNumRoutes(); i++)
    //    if (rt->getRoute(i)->host == dest)
    //        break;
    //
    //if (i == rt->getNumRoutes())
    //    return IPv4Address();  // Signal an NOTIFICATION of NO ROUTE
    //
    InterfaceEntry *ie = rt->getInterfaceForDestAddr(dest);
    if (!ie)
        return IPv4Address(); // no route

    std::string iName = ie->getName();    // FIXME why use name for lookup?
    return findPeerAddrFromInterface(iName);
}

// FIXME To allow this to work, make sure there are entries of hosts for all peers

IPv4Address LDP::findPeerAddrFromInterface(std::string interfaceName)
{
    int i = 0;
    int k = 0;
    InterfaceEntry *ie = ift->getInterfaceByName(interfaceName.c_str());

    const IPv4Route *anEntry;

    for (i = 0; i < rt->getNumRoutes(); i++) {
        for (k = 0; k < (int)myPeers.size(); k++) {
            anEntry = rt->getRoute(i);
            if (anEntry->getDestination() == myPeers[k].peerIP && anEntry->getInterface() == ie) {
                return myPeers[k].peerIP;
            }
            // addresses->push_back(peerIP[k]);
        }
    }

    // Return any IP which has default route - not in routing table entries
    for (i = 0; i < (int)myPeers.size(); i++) {
        for (k = 0; k < rt->getNumRoutes(); k++) {
            anEntry = rt->getRoute(i);
            if (anEntry->getDestination() == myPeers[i].peerIP)
                break;
        }
        if (k == rt->getNumRoutes())
            break;
    }

    // return the peer's address if found, unspecified address otherwise
    return i == (int)myPeers.size() ? IPv4Address() : myPeers[i].peerIP;
}

// Pre-condition: myPeers vector is finalized
std::string LDP::findInterfaceFromPeerAddr(IPv4Address peerIP)
{
/*
    int i;
    for (unsigned int i=0;i<myPeers.size();i++)
    {
        if (myPeers[i].peerIP == peerIP)
            return string(myPeers[i].linkInterface);
    }
    return string("X");
 */
//    Rely on port index to find the interface name

    // this function is a misnomer, we must recognize our own address too
    if (rt->isLocalAddress(peerIP))
        return "lo0";

    InterfaceEntry *ie = rt->getInterfaceForDestAddr(peerIP);
    if (!ie)
        throw cRuntimeError("findInterfaceFromPeerAddr(): %s is not routable", peerIP.str().c_str());
    return ie->getName();
}

//bool LDP::matches(const FEC_TLV& a, const FEC_TLV& b)
//{
//  return b.addr.prefixMatches(a, b.length);
//}

LDP::FecBindVector::iterator LDP::findFecEntry(FecBindVector& fecs, int fecid, IPv4Address peer)
{
    auto it = fecs.begin();
    for (; it != fecs.end(); it++) {
        if ((it->fecid == fecid) && (it->peer == peer))
            break;
    }
    return it;
}

LDP::FecVector::iterator LDP::findFecEntry(FecVector& fecs, IPv4Address addr, int length)
{
    auto it = fecs.begin();
    for ( ; it != fecs.end(); it++) {
        if ((it->length == length) && (it->addr == addr)) // XXX compare only relevant part (?)
            break;
    }
    return it;
}

void LDP::sendNotify(int status, IPv4Address dest, IPv4Address addr, int length)
{
    // Send NOTIFY message
    LDPNotify *lnMessage = new LDPNotify("Lb-Notify");
    lnMessage->setByteLength(LDP_HEADER_BYTES);    // FIXME find out actual length
    lnMessage->setType(NOTIFICATION);
    lnMessage->setStatus(NO_ROUTE);
    lnMessage->setReceiverAddress(dest);
    lnMessage->setSenderAddress(rt->getRouterId());

    FEC_TLV fec;
    fec.addr = addr;
    fec.length = length;

    lnMessage->setFec(fec);

    sendToPeer(dest, lnMessage);
}

void LDP::sendMapping(int type, IPv4Address dest, int label, IPv4Address addr, int length)
{
    // Send LABEL MAPPING downstream
    LDPLabelMapping *lmMessage = new LDPLabelMapping("Lb-Mapping");
    lmMessage->setByteLength(LDP_HEADER_BYTES);    // FIXME find out actual length
    lmMessage->setType(type);
    lmMessage->setReceiverAddress(dest);
    lmMessage->setSenderAddress(rt->getRouterId());
    lmMessage->setLabel(label);

    FEC_TLV fec;
    fec.addr = addr;
    fec.length = length;

    lmMessage->setFec(fec);

    sendToPeer(dest, lmMessage);
}

void LDP::processNOTIFICATION(LDPNotify *packet)
{
    FEC_TLV fec = packet->getFec();
    IPv4Address srcAddr = packet->getSenderAddress();
    int status = packet->getStatus();

    // XXX FIXME NO_ROUTE processing should probably be split into two functions,
    // this is not the cleanest thing I ever wrote :)   --Vojta

    if (packet->isSelfMessage()) {
        // re-scheduled by ourselves
        EV_INFO << "notification retry for peer=" << srcAddr << " fec=" << fec << " status=" << status << endl;
    }
    else {
        // received via network
        EV_INFO << "notification received from=" << srcAddr << " fec=" << fec << " status=" << status << endl;
    }

    switch (status) {
        case NO_ROUTE: {
            EV_INFO << "route does not exit on that peer" << endl;

            auto it = findFecEntry(fecList, fec.addr, fec.length);
            if (it != fecList.end()) {
                if (it->nextHop == srcAddr) {
                    if (!packet->isSelfMessage()) {
                        EV_DETAIL << "we are still interesed in this mapping, we will retry later" << endl;

                        scheduleAt(simTime() + 1.0    /* XXX FIXME */, packet);
                        return;
                    }
                    else {
                        EV_DETAIL << "reissuing request" << endl;

                        sendMappingRequest(srcAddr, fec.addr, fec.length);
                    }
                }
                else
                    EV_DETAIL << "and we still recognize this FEC, but we use different next hop, forget it" << endl;
            }
            else
                EV_DETAIL << "and we do not recognize this any longer, forget it" << endl;

            break;
        }

        default:
            ASSERT(false);
            break;
    }

    delete packet;
}

void LDP::processLABEL_REQUEST(LDPLabelRequest *packet)
{
    FEC_TLV fec = packet->getFec();
    IPv4Address srcAddr = packet->getSenderAddress();

    EV_INFO << "Label Request from LSR " << srcAddr << " for FEC " << fec << endl;

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        EV_DETAIL << "FEC not recognized, sending back No route message" << endl;

        sendNotify(NO_ROUTE, srcAddr, fec.addr, fec.length);

        delete packet;
        return;
    }

    // do we already have mapping for this fec from our downstream peer?

    //
    // XXX this code duplicates rebuildFecList
    //

    // does upstream have mapping from us?
    auto uit = findFecEntry(fecUp, it->fecid, srcAddr);

    // shouldn't!
    ASSERT(uit == fecUp.end());

    // do we have mapping from downstream?
    auto dit = findFecEntry(fecDown, it->fecid, it->nextHop);

    // is next hop our LDP peer?
    bool ER = !findPeerSocket(it->nextHop);

    ASSERT(!(ER && dit != fecDown.end()));    // can't be egress and have mapping at the same time

    if (ER || dit != fecDown.end()) {
        fec_bind_t newItem;
        newItem.fecid = it->fecid;
        newItem.label = -1;
        newItem.peer = srcAddr;
        fecUp.push_back(newItem);
        uit = fecUp.end() - 1;
    }

    std::string inInterface = findInterfaceFromPeerAddr(srcAddr);
    std::string outInterface = findInterfaceFromPeerAddr(it->nextHop);

    if (ER) {
        // we are egress, that's easy:
        LabelOpVector outLabel = LIBTable::popLabel();

        uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, 0);

        EV_DETAIL << "installed (egress) LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                  << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        // We are egress, let our upstream peer know
        // about it by sending back a Label Mapping message

        sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);
    }
    else if (dit != fecDown.end()) {
        // we have mapping from DS, that's easy
        LabelOpVector outLabel = LIBTable::swapLabel(dit->label);
        uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);

        EV_DETAIL << "installed LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                  << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        // We already have a mapping for this FEC, let our upstream peer know
        // about it by sending back a Label Mapping message

        sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);
    }
    else {
        // no mapping from DS, mark as pending

        EV_DETAIL << "no mapping for this FEC from the downstream router, marking as pending" << endl;

        pending_req_t newItem;
        newItem.fecid = it->fecid;
        newItem.peer = srcAddr;
        pending.push_back(newItem);
    }

    delete packet;
}

void LDP::processLABEL_RELEASE(LDPLabelMapping *packet)
{
    FEC_TLV fec = packet->getFec();
    int label = packet->getLabel();
    IPv4Address fromIP = packet->getSenderAddress();

    EV_INFO << "Mapping release received for label=" << label << " fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    // remove label from fecUp

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        EV_INFO << "FEC no longer recognized here, ignoring" << endl;
        delete packet;
        return;
    }

    auto uit = findFecEntry(fecUp, it->fecid, fromIP);
    if (uit == fecUp.end() || label != uit->label) {
        // this is ok and may happen; e.g. we removed the mapping because downstream
        // neighbour withdrew its mapping. we sent withdraw upstream as well and
        // this is upstream's response
        EV_INFO << "mapping not found among sent mappings, ignoring" << endl;
        delete packet;
        return;
    }

    EV_DETAIL << "removing from LIB table label=" << uit->label << endl;
    lt->removeLibEntry(uit->label);

    EV_DETAIL << "removing label from list of sent mappings" << endl;
    fecUp.erase(uit);

    delete packet;
}

void LDP::processLABEL_WITHDRAW(LDPLabelMapping *packet)
{
    FEC_TLV fec = packet->getFec();
    int label = packet->getLabel();
    IPv4Address fromIP = packet->getSenderAddress();

    EV_INFO << "Mapping withdraw received for label=" << label << " fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    // remove label from fecDown

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        EV_INFO << "matching FEC not found, ignoring withdraw message" << endl;
        delete packet;
        return;
    }

    auto dit = findFecEntry(fecDown, it->fecid, fromIP);

    if (dit == fecDown.end() || label != dit->label) {
        EV_INFO << "matching mapping not found, ignoring withdraw message" << endl;
        delete packet;
        return;
    }

    ASSERT(dit != fecDown.end());
    ASSERT(label == dit->label);

    EV_INFO << "removing label from list of received mappings" << endl;
    fecDown.erase(dit);

    EV_INFO << "sending back relase message" << endl;
    packet->setType(LABEL_RELEASE);

    // send msg to peer over TCP
    sendToPeer(fromIP, packet);

    updateFecListEntry(*it);
}

void LDP::processLABEL_MAPPING(LDPLabelMapping *packet)
{
    FEC_TLV fec = packet->getFec();
    int label = packet->getLabel();
    IPv4Address fromIP = packet->getSenderAddress();

    EV_INFO << "Label mapping label=" << label << " received for fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end())
        throw cRuntimeError("Model error: fec not in fecList");

    auto dit = findFecEntry(fecDown, it->fecid, fromIP);
    if (dit != fecDown.end())
        throw cRuntimeError("Model error: found in fecDown");

    // insert among received mappings

    fec_bind_t newItem;
    newItem.fecid = it->fecid;
    newItem.peer = fromIP;
    newItem.label = label;
    fecDown.push_back(newItem);

    // respond to pending requests

    for (auto pit = pending.begin(); pit != pending.end(); ) {
        if (pit->fecid != it->fecid) {
            pit++;
            continue;
        }

        EV_DETAIL << "there's pending request for this FEC from " << pit->peer << ", sending mapping" << endl;

        std::string inInterface = findInterfaceFromPeerAddr(pit->peer);
        std::string outInterface = findInterfaceFromPeerAddr(fromIP);
        LabelOpVector outLabel = LIBTable::swapLabel(label);

        fec_bind_t newItem;
        newItem.fecid = it->fecid;
        newItem.peer = pit->peer;
        newItem.label = lt->installLibEntry(-1, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);
        fecUp.push_back(newItem);

        EV_DETAIL << "installed LIB entry inLabel=" << newItem.label << " inInterface=" << inInterface
                  << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        sendMapping(LABEL_MAPPING, pit->peer, newItem.label, it->addr, it->length);

        // remove request from the list
        pit = pending.erase(pit);
    }

    delete packet;
}

int LDP::findPeer(IPv4Address peerAddr)
{
    for (auto i = myPeers.begin(); i != myPeers.end(); ++i)
        if (i->peerIP == peerAddr)
            return i - myPeers.begin();

    return -1;
}

TCPSocket *LDP::findPeerSocket(IPv4Address peerAddr)
{
    // find peer in table and return its socket
    int i = findPeer(peerAddr);
    if (i == -1 || !(myPeers[i].socket) || myPeers[i].socket->getState() != TCPSocket::CONNECTED)
        return nullptr; // we don't have an LDP session to this peer
    return myPeers[i].socket;
}

TCPSocket *LDP::getPeerSocket(IPv4Address peerAddr)
{
    TCPSocket *sock = findPeerSocket(peerAddr);
    ASSERT(sock);
    if (!sock)
        throw cRuntimeError("No LDP session to peer %s yet", peerAddr.str().c_str());
    return sock;
}

bool LDP::lookupLabel(IPv4Datagram *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color)
{
    IPv4Address destAddr = ipdatagram->getDestAddress();
    int protocol = ipdatagram->getTransportProtocol();

    // never match and always route via L3 if:

    // OSPF traffic (TED)
    if (protocol == IP_PROT_OSPF)
        return false;

    // LDP traffic (both discovery...
    if (protocol == IP_PROT_UDP && check_and_cast<UDPPacket *>(ipdatagram->getEncapsulatedPacket())->getDestinationPort() == LDP_PORT)
        return false;

    // ...and session)
    if (protocol == IP_PROT_TCP && check_and_cast<tcp::TCPSegment *>(ipdatagram->getEncapsulatedPacket())->getDestPort() == LDP_PORT)
        return false;
    if (protocol == IP_PROT_TCP && check_and_cast<tcp::TCPSegment *>(ipdatagram->getEncapsulatedPacket())->getSrcPort() == LDP_PORT)
        return false;

    // regular traffic, classify, label etc.

    for (auto & elem : fecList) {
        if (!destAddr.prefixMatches(elem.addr, elem.length))
            continue;

        EV_DETAIL << "FEC matched: " << elem << endl;

        auto dit = findFecEntry(fecDown, elem.fecid, elem.nextHop);
        if (dit != fecDown.end()) {
            outLabel = LIBTable::pushLabel(dit->label);
            outInterface = findInterfaceFromPeerAddr(elem.nextHop);
            color = LDP_USER_TRAFFIC;
            EV_DETAIL << "mapping found, outLabel=" << outLabel << ", outInterface=" << outInterface << endl;
            return true;
        }
        else {
            EV_DETAIL << "no mapping for this FEC exists" << endl;
            return false;
        }
    }
    return false;
}

void LDP::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    ASSERT(signalID == NF_ROUTE_ADDED || signalID == NF_ROUTE_DELETED);

    EV_INFO << "routing table changed, rebuild list of known FEC" << endl;

    rebuildFecList();
}

void LDP::announceLinkChange(int tedlinkindex)
{
    TEDChangeInfo d;
    d.setTedLinkIndicesArraySize(1);
    d.setTedLinkIndices(0, tedlinkindex);
    emit(NF_TED_CHANGED, &d);
}

} // namespace inet

