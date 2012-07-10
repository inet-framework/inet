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

#include "INETDefs.h"

#include "LDP.h"

//#include "ConstType.h"
#include "LIBTable.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "RoutingTableAccess.h"
#include "LIBTableAccess.h"
#include "TEDAccess.h"
#include "NotifierConsts.h"
#include "UDPControlInfo_m.h"
#include "UDPPacket.h"
#include "TCPSegment.h"


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
    os << "peerIP=" << p.peerIP << "  interface=" << p.linkInterface <<
          "  activeRole=" << (p.activeRole ? "true" : "false") <<
          "  socket=" << (p.socket ? TCPSocket::stateName(p.socket->getState()) : "NULL");
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
    sendHelloMsg = NULL;
}

LDP::~LDP()
{
    for (unsigned int i=0; i<myPeers.size(); i++)
        cancelAndDelete(myPeers[i].timeout);

    cancelAndDelete(sendHelloMsg);
    //this causes segfault at the end of simulation       -- Vojta
    //socketMap.deleteSockets();
}

void LDP::initialize(int stage)
{
    if (stage != 3)
        return; // wait for routing table to initialize first

    holdTime = par("holdTime").doubleValue();
    helloInterval = par("helloInterval").doubleValue();

    ift = InterfaceTableAccess().get();
    rt = RoutingTableAccess().get();
    lt = LIBTableAccess().get();
    tedmod = TEDAccess().get();
    nb = NotificationBoardAccess().get();

    WATCH_VECTOR(myPeers);
    WATCH_VECTOR(fecUp);
    WATCH_VECTOR(fecDown);
    WATCH_VECTOR(fecList);
    WATCH_VECTOR(pending);

    maxFecid = 0;

    // schedule first hello
    sendHelloMsg = new cMessage("LDPSendHello");
    scheduleAt(simTime() + exponential(0.1), sendHelloMsg);

    // bind UDP socket
    udpSocket.setOutputGate(gate("udpOut"));
    udpSocket.bind(LDP_PORT);
    for (int i = 0; i < ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->isMulticast())
        {
            udpSockets.push_back(UDPSocket());
            udpSockets.back().setOutputGate(gate("udpOut"));
            udpSockets.back().setMulticastLoop(false);
            udpSockets.back().setMulticastOutputInterface(ie->getInterfaceId());
        }
    }

    // start listening for incoming TCP conns
    EV << "Starting to listen on port " << LDP_PORT << " for incoming LDP sessions\n";
    serverSocket.setOutputGate(gate("tcpOut"));
    serverSocket.readDataTransferModePar(*this);
    serverSocket.bind(LDP_PORT);
    serverSocket.listen();

    // build list of recognized FECs
    rebuildFecList();

    // listen for routing table modifications
    nb->subscribe(this, NF_IPv4_ROUTE_ADDED);
    nb->subscribe(this, NF_IPv4_ROUTE_DELETED);
}

void LDP::handleMessage(cMessage *msg)
{
    EV << "Received: (" << msg->getClassName() << ")" << msg->getName() << "\n";
    if (msg==sendHelloMsg)
    {
        // every LDP capable router periodically sends HELLO messages to the
        // "all routers in the sub-network" multicast address
        EV << "Multicasting LDP Hello to neighboring routers\n";
        sendHelloTo(IPv4Address::ALL_ROUTERS_MCAST);

        // schedule next hello
        scheduleAt(simTime() + helloInterval, sendHelloMsg);
    }
    else if (msg->isSelfMessage())
    {
        EV << "Timer " << msg->getName() << " expired\n";
        if (!strcmp(msg->getName(), "HelloTimeout"))
        {
            processHelloTimeout(msg);
        }
        else
        {
            processNOTIFICATION(check_and_cast<LDPNotify*>(msg));
        }
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "udpIn"))
    {
        // we can only receive LDP Hello from UDP (everything else goes over TCP)
        processLDPHello(check_and_cast<LDPHello *>(msg));
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "tcpIn"))
    {
        processMessageFromTCP(msg);
    }
}

void LDP::sendToPeer(IPv4Address dest, cMessage *msg)
{
    getPeerSocket(dest)->send(msg);
}

void LDP::sendMappingRequest(IPv4Address dest, IPv4Address addr, int length)
{
    LDPLabelRequest *requestMsg = new LDPLabelRequest("Lb-Req");
    requestMsg->setByteLength(LDP_HEADER_BYTES); // FIXME find out actual length
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
    FecBindVector::iterator dit = findFecEntry(fecDown, oldItem.fecid, oldItem.nextHop);

    // is next hop our LDP peer?
    bool ER = findPeerSocket(oldItem.nextHop)==NULL;

    ASSERT(!(ER && dit != fecDown.end())); // can't be egress and have mapping at the same time

    // adjust upstream mappings
    FecBindVector::iterator uit;
    for (uit = fecUp.begin(); uit != fecUp.end();)
    {
        if (uit->fecid != oldItem.fecid)
        {
            uit++;
            continue;
        }

        std::string inInterface = findInterfaceFromPeerAddr(uit->peer);
        std::string outInterface = findInterfaceFromPeerAddr(oldItem.nextHop);
        if (ER)
        {
            // we are egress, that's easy:
            LabelOpVector outLabel = LIBTable::popLabel();
            uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);

            EV << "installed (egress) LIB entry inLabel=" << uit->label << " inInterface=" << inInterface <<
                    " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
            uit++;
        }
        else if (dit != fecDown.end())
        {
            // we have mapping from DS, that's easy
            LabelOpVector outLabel = LIBTable::swapLabel(dit->label);
            uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);

            EV << "installed LIB entry inLabel=" << uit->label << " inInterface=" << inInterface <<
                    " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
            uit++;
        }
        else
        {
            // no mapping from DS, withdraw mapping US
            EV << "sending withdraw message upstream" << endl;
            sendMapping(LABEL_WITHDRAW, uit->peer, uit->label, oldItem.addr, oldItem.length);

            // remove from US mappings
            uit = fecUp.erase(uit);
        }
    }

    if (!ER && dit == fecDown.end())
    {
        // and ask DS for mapping
        EV << "sending request message downstream" << endl;
        sendMappingRequest(oldItem.nextHop, oldItem.addr, oldItem.length);
    }
}

void LDP::rebuildFecList()
{
    EV << "make list of recognized FECs" << endl;

    FecVector oldList = fecList;
    fecList.clear();

    for (int i = 0; i < rt->getNumRoutes(); i++)
    {
        // every entry in the routing table

        const IPv4Route *re = rt->getRoute(i);

        // ignore multicast routes
        if (re->getDestination().isMulticast())
            continue;

        // find out current next hop according to routing table
        IPv4Address nextHop = (re->getGateway().isUnspecified()) ? re->getDestination() : re->getGateway();
        ASSERT(!nextHop.isUnspecified());

        EV << "nextHop <-- " << nextHop << endl;

        FecVector::iterator it = findFecEntry(oldList, re->getDestination(), re->getNetmask().getNetmaskLength());

        if (it == oldList.end())
        {
            // fec didn't exist, it was just created
            fec_t newItem;
            newItem.fecid = ++maxFecid;
            newItem.addr = re->getDestination();
            newItem.length = re->getNetmask().getNetmaskLength();
            newItem.nextHop = nextHop;
            updateFecListEntry(newItem);
            fecList.push_back(newItem);
        }
        else if (it->nextHop != nextHop)
        {
            // next hop for this FEC changed,
            it->nextHop = nextHop;
            updateFecListEntry(*it);
            fecList.push_back(*it);
            oldList.erase(it);
        }
        else
        {
            // FEC didn't change, reusing old values
            fecList.push_back(*it);
            oldList.erase(it);
            continue;
        }
    }


    // our own addresses (XXX is it needed?)

    for (int i = 0; i < ift->getNumInterfaces(); ++i)
    {
        InterfaceEntry *ie = ift->getInterface(i);
        if (ie->getNetworkLayerGateIndex() < 0)
            continue;

        FecVector::iterator it = findFecEntry(oldList, ie->ipv4Data()->getIPAddress(), 32);
        if (it == oldList.end())
        {
            fec_t newItem;
            newItem.fecid = ++maxFecid;
            newItem.addr = ie->ipv4Data()->getIPAddress();
            newItem.length = 32;
            newItem.nextHop = ie->ipv4Data()->getIPAddress();
            fecList.push_back(newItem);
        }
        else
        {
            fecList.push_back(*it);
            oldList.erase(it);
        }
    }

    if (oldList.size() > 0)
    {
        EV << "there are " << oldList.size() << " deprecated FECs, removing them" << endl;

        FecVector::iterator it;
        for (it = oldList.begin(); it != oldList.end(); it++)
        {
            EV << "removing FEC= " << *it << endl;

            FecBindVector::iterator dit;
            for (dit = fecDown.begin(); dit != fecDown.end(); dit++)
            {
                if (dit->fecid != it->fecid)
                    continue;

                EV << "sending release label=" << dit->label << " downstream to " << dit->peer << endl;

                sendMapping(LABEL_RELEASE, dit->peer, dit->label, it->addr, it->length);
            }

            FecBindVector::iterator uit;
            for (uit = fecUp.begin(); uit != fecUp.end(); uit++)
            {
                if (uit->fecid != it->fecid)
                    continue;

                EV << "sending withdraw label=" << uit->label << " upstream to " << uit->peer << endl;

                sendMapping(LABEL_WITHDRAW, uit->peer, uit->label, it->addr, it->length);

                EV << "removing entry inLabel=" << uit->label << " from LIB" << endl;

                lt->removeLibEntry(uit->label);
            }

        }
    }

    // we must keep this list sorted for matching to work correctly
    // this is probably slower than it must be
    std::sort(fecList.begin(), fecList.end(), fecPrefixCompare);
}

void LDP::updateFecList(IPv4Address nextHop)
{
    FecVector::iterator it;
    for (it = fecList.begin(); it != fecList.end(); it++)
    {
        if (it->nextHop != nextHop)
            continue;

        updateFecListEntry(*it);
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

    if (dest.isMulticast())
    {
        for (int i = 0; i < (int)udpSockets.size(); ++i)
        {
            LDPHello *msg = i== (int)udpSockets.size() - 1 ? hello : hello->dup();
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

    EV << "peer=" << peerIP << " is gone, removing adjacency" << endl;

    ASSERT(!myPeers[i].timeout->isScheduled());
    delete myPeers[i].timeout;
    ASSERT(myPeers[i].socket);
    myPeers[i].socket->abort(); // should we only close?
    delete myPeers[i].socket;
    myPeers.erase(myPeers.begin() + i);

    EV << "removing (stale) bindings from fecDown for peer=" << peerIP << endl;

    FecBindVector::iterator dit;
    for (dit = fecDown.begin(); dit != fecDown.end();)
    {
        if (dit->peer != peerIP)
        {
            dit++;
            continue;
        }

        EV << "label=" << dit->label << endl;

        // send release message just in case (?)
        // what happens if peer is not really down and
        // hello messages just disappeared?
        // does the protocol recover on its own (XXX check this)

        dit = fecDown.erase(dit);
    }

    EV << "removing bindings from sent to peer=" << peerIP << " from fecUp" << endl;

    FecBindVector::iterator uit;
    for (uit = fecUp.begin(); uit != fecUp.end();)
    {
        if (uit->peer != peerIP)
        {
            uit++;
            continue;
        }

        EV << "label=" << uit->label << endl;

        // send withdraw message just in case (?)
        // see comment above...

        uit = fecUp.erase(uit);
    }

    EV << "updating fecList" << endl;

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
    //IPv4Address peerAddr = controlInfo->getSrcAddr().get4();
    IPv4Address peerAddr = msg->getSenderAddress();
    int interfaceId = controlInfo->getInterfaceId();
    delete msg;

    EV << "Received LDP Hello from " << peerAddr << ", ";

    if (peerAddr.isUnspecified() || peerAddr==rt->getRouterId())
    {
        // must be ourselves (we're also in the all-routers multicast group), ignore
        EV << "that's myself, ignore\n";
        return;
    }

    // mark link as working if it was failed, and rebuild table
    unsigned int index = tedmod->linkIndex(rt->getRouterId(), peerAddr);
    if (!tedmod->ted[index].state)
    {
        tedmod->ted[index].state = true;
        tedmod->rebuildRoutingTable();
        announceLinkChange(index);
    }

    // peer already in table?
    int i = findPeer(peerAddr);
    if (i!=-1)
    {
        EV << "already in my peer table, rescheduling timeout" << endl;
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
    info.socket = NULL;
    info.timeout = new cMessage("HelloTimeout");
    scheduleAt(simTime() + holdTime, info.timeout);
    myPeers.push_back(info);
    int peerIndex = myPeers.size()-1;

    EV << "added to peer table\n";
    EV << "We'll be " << (info.activeRole ? "ACTIVE" : "PASSIVE") << " in this session\n";

    // introduce ourselves with a Hello, then connect if we're in ACTIVE role
    sendHelloTo(peerAddr);
    if (info.activeRole)
    {
        EV << "Establishing session with it\n";
        openTCPConnectionToPeer(peerIndex);
    }
}

void LDP::openTCPConnectionToPeer(int peerIndex)
{
    TCPSocket *socket = new TCPSocket();
    socket->setOutputGate(gate("tcpOut"));
    socket->setCallbackObject(this, (void*)peerIndex);
    socket->readDataTransferModePar(*this);
    socket->bind(rt->getRouterId(), 0);
    socketMap.addSocket(socket);
    myPeers[peerIndex].socket = socket;

    socket->connect(myPeers[peerIndex].peerIP, LDP_PORT);
}

void LDP::processMessageFromTCP(cMessage *msg)
{
    TCPSocket *socket = socketMap.findSocketFor(msg);
    if (!socket)
    {
        // not yet in socketMap, must be new incoming connection.
        // find which peer it is and register connection
        socket = new TCPSocket(msg);
        socket->setOutputGate(gate("tcpOut"));
        socket->readDataTransferModePar(*this);

        // FIXME there seems to be some confusion here. Is it sure that
        // routerIds we use as peerAddrs are the same as IP addresses
        // the routing is based on? --Andras
        IPv4Address peerAddr = socket->getRemoteAddress().get4();

        int i = findPeer(peerAddr);
        if (i==-1 || myPeers[i].socket)
        {
            // nothing known about this guy, or already connected: refuse
            socket->close(); // reset()?
            delete socket;
            delete msg;
            return;
        }
        myPeers[i].socket = socket;
        socket->setCallbackObject(this, (void *)i);
        socketMap.addSocket(socket);
    }

    // dispatch to socketEstablished(), socketDataArrived(), socketPeerClosed()
    // or socketFailure()
    socket->processMessage(msg);
}

void LDP::socketEstablished(int, void *yourPtr)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV << "TCP connection established with peer " << peer.peerIP << "\n";

    // we must update all entries with nextHop == peerIP
    updateFecList(peer.peerIP);

    // FIXME start LDP session setup (if we're on the active side?)
}

void LDP::socketDataArrived(int, void *yourPtr, cPacket *msg, bool)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV << "Message arrived over TCP from peer " << peer.peerIP << "\n";

    delete msg->removeControlInfo();
    processLDPPacketFromTCP(check_and_cast<LDPPacket *>(msg));
}

void LDP::socketPeerClosed(int, void *yourPtr)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV << "Peer " << peer.peerIP << " closed TCP connection\n";

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
    EV << "TCP connection to peer " << peer.peerIP << " closed\n";

    ASSERT(false);

    // FIXME what now? reconnect after a delay?
}

void LDP::socketFailure(int, void *yourPtr, int code)
{
    peer_info& peer = myPeers[(long)yourPtr];
    EV << "TCP connection to peer " << peer.peerIP << " broken\n";

    ASSERT(false);

    // FIXME what now? reconnect after a delay?
}

void LDP::processLDPPacketFromTCP(LDPPacket *ldpPacket)
{
    switch (ldpPacket->getType())
    {
    case HELLO:
        error("Received LDP HELLO over TCP (should arrive over UDP)");
        break;

    case ADDRESS:
        // processADDRESS(ldpPacket);
        error("Received LDP ADDRESS message, unsupported in this version");
        break;

    case ADDRESS_WITHDRAW:
        // processADDRESS_WITHDRAW(ldpPacket);
        error("LDP PROC DEBUG: Received LDP ADDRESS_WITHDRAW message, unsupported in this version");
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
        processNOTIFICATION(check_and_cast<LDPNotify*>(ldpPacket));
        break;

    default:
        error("LDP PROC DEBUG: Unrecognized LDP Message Type, type is %d", ldpPacket->getType());
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
        return IPv4Address();  // no route

    std::string iName = ie->getName(); // FIXME why use name for lookup?
    return findPeerAddrFromInterface(iName);
}

// FIXME To allow this to work, make sure there are entries of hosts for all peers

IPv4Address LDP::findPeerAddrFromInterface(std::string interfaceName)
{
    int i = 0;
    int k = 0;
    InterfaceEntry *ie = ift->getInterfaceByName(interfaceName.c_str());

    const IPv4Route *anEntry;

    for (i = 0; i < rt->getNumRoutes(); i++)
    {
        for (k = 0; k < (int)myPeers.size(); k++)
        {
            anEntry = rt->getRoute(i);
            if (anEntry->getDestination()==myPeers[k].peerIP && anEntry->getInterface()==ie)
            {
                return myPeers[k].peerIP;
            }
            // addresses->push_back(peerIP[k]);
        }
    }

    // Return any IP which has default route - not in routing table entries
    for (i = 0; i < (int)myPeers.size(); i++)
    {
        for (k = 0; k < rt->getNumRoutes(); k++)
        {
            anEntry = rt->getRoute(i);
            if (anEntry->getDestination() == myPeers[i].peerIP)
                break;
        }
        if (k == rt->getNumRoutes())
            break;
    }

    // return the peer's address if found, unspecified address otherwise
    return i==(int)myPeers.size() ? IPv4Address() : myPeers[i].peerIP;
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
        error("findInterfaceFromPeerAddr(): %s is not routable", peerIP.str().c_str());
    return ie->getName();
}

//bool LDP::matches(const FEC_TLV& a, const FEC_TLV& b)
//{
//  return b.addr.prefixMatches(a, b.length);
//}

LDP::FecBindVector::iterator LDP::findFecEntry(FecBindVector& fecs, int fecid, IPv4Address peer)
{
    FecBindVector::iterator it;
    for (it = fecs.begin(); it != fecs.end(); it++)
    {
        if (it->fecid != fecid)
            continue;

        if (it->peer != peer)
            continue;

        break;
    }
    return it;
}

LDP::FecVector::iterator LDP::findFecEntry(FecVector& fecs, IPv4Address addr, int length)
{
    FecVector::iterator it;
    for (it = fecs.begin(); it != fecs.end(); it++)
    {
        if (it->length != length)
            continue;

        if (it->addr != addr) // XXX compare only relevant part (?)
            continue;

        break;
    }
    return it;
}

void LDP::sendNotify(int status, IPv4Address dest, IPv4Address addr, int length)
{
    // Send NOTIFY message
    LDPNotify *lnMessage = new LDPNotify("Lb-Notify");
    lnMessage->setByteLength(LDP_HEADER_BYTES); // FIXME find out actual length
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
    lmMessage->setByteLength(LDP_HEADER_BYTES); // FIXME find out actual length
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

    if (packet->isSelfMessage())
    {
        // re-scheduled by ourselves
        EV << "notification retry for peer=" << srcAddr << " fec=" << fec << " status=" << status << endl;
    }
    else
    {
        // received via network
        EV << "notification received from=" << srcAddr << " fec=" << fec << " status=" << status << endl;
    }

    switch (status)
    {
        case NO_ROUTE:
        {
            EV << "route does not exit on that peer" << endl;

            FecVector::iterator it = findFecEntry(fecList, fec.addr, fec.length);
            if (it != fecList.end())
            {
                if (it->nextHop == srcAddr)
                {
                    if (!packet->isSelfMessage())
                    {
                        EV << "we are still interesed in this mapping, we will retry later" << endl;

                        scheduleAt(simTime() + 1.0 /* XXX FIXME */, packet);
                        return;
                    }
                    else
                    {
                        EV << "reissuing request" << endl;

                        sendMappingRequest(srcAddr, fec.addr, fec.length);
                    }
                }
                else
                    EV << "and we still recognize this FEC, but we use different next hop, forget it" << endl;
            }
            else
                EV << "and we do not recognize this any longer, forget it" << endl;

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

    EV << "Label Request from LSR " << srcAddr << " for FEC " << fec << endl;

    FecVector::iterator it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end())
    {
        EV << "FEC not recognized, sending back No route message" << endl;

        sendNotify(NO_ROUTE, srcAddr, fec.addr, fec.length);

        delete packet;
        return;
    }

    // do we already have mapping for this fec from our downstream peer?

    //
    // XXX this code duplicates rebuildFecList
    //

    // does upstream have mapping from us?
    FecBindVector::iterator uit = findFecEntry(fecUp, it->fecid, srcAddr);

    // shouldn't!
    ASSERT(uit == fecUp.end());

    // do we have mapping from downstream?
    FecBindVector::iterator dit = findFecEntry(fecDown, it->fecid, it->nextHop);

    // is next hop our LDP peer?
    bool ER = !findPeerSocket(it->nextHop);

    ASSERT(!(ER && dit != fecDown.end())); // can't be egress and have mapping at the same time

    if (ER || dit != fecDown.end())
    {
        fec_bind_t newItem;
        newItem.fecid = it->fecid;
        newItem.label = -1;
        newItem.peer = srcAddr;
        fecUp.push_back(newItem);
        uit = fecUp.end() - 1;
    }

    std::string inInterface = findInterfaceFromPeerAddr(srcAddr);
    std::string outInterface = findInterfaceFromPeerAddr(it->nextHop);

    if (ER)
    {
        // we are egress, that's easy:
        LabelOpVector outLabel = LIBTable::popLabel();

        uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, 0);

        EV << "installed (egress) LIB entry inLabel=" << uit->label << " inInterface=" << inInterface <<
                " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        // We are egress, let our upstream peer know
        // about it by sending back a Label Mapping message

        sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);

    }
    else if (dit != fecDown.end())
    {
        // we have mapping from DS, that's easy
        LabelOpVector outLabel = LIBTable::swapLabel(dit->label);
        uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);

        EV << "installed LIB entry inLabel=" << uit->label << " inInterface=" << inInterface <<
                " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        // We already have a mapping for this FEC, let our upstream peer know
        // about it by sending back a Label Mapping message

        sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);
    }
    else
    {
        // no mapping from DS, mark as pending

        EV << "no mapping for this FEC from the downstream router, marking as pending" << endl;

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

    EV << "Mapping release received for label=" << label << " fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    // remove label from fecUp

    FecVector::iterator it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end())
    {
        EV << "FEC no longer recognized here, ignoring" << endl;
        delete packet;
        return;
    }

    FecBindVector::iterator uit = findFecEntry(fecUp, it->fecid, fromIP);
    if (uit == fecUp.end() || label != uit->label)
    {
        // this is ok and may happen; e.g. we removed the mapping because downstream
        // neighbour withdrew its mapping. we sent withdraw upstream as well and
        // this is upstream's response
        EV << "mapping not found among sent mappings, ignoring" << endl;
        delete packet;
        return;
    }

    EV << "removing from LIB table label=" << uit->label << endl;
    lt->removeLibEntry(uit->label);

    EV << "removing label from list of sent mappings" << endl;
    fecUp.erase(uit);

    delete packet;
}

void LDP::processLABEL_WITHDRAW(LDPLabelMapping *packet)
{
    FEC_TLV fec = packet->getFec();
    int label = packet->getLabel();
    IPv4Address fromIP = packet->getSenderAddress();

    EV << "Mapping withdraw received for label=" << label << " fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    // remove label from fecDown

    FecVector::iterator it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end())
    {
        EV << "matching FEC not found, ignoring withdraw message" << endl;
        delete packet;
        return;
    }

    FecBindVector::iterator dit = findFecEntry(fecDown, it->fecid, fromIP);

    if (dit == fecDown.end() || label != dit->label)
    {
        EV << "matching mapping not found, ignoring withdraw message" << endl;
        delete packet;
        return;
    }

    ASSERT(dit != fecDown.end());
    ASSERT(label == dit->label);

    EV << "removing label from list of received mappings" << endl;
    fecDown.erase(dit);

    EV << "sending back relase message" << endl;
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

    EV << "Label mapping label=" << label << " received for fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    FecVector::iterator it = findFecEntry(fecList, fec.addr, fec.length);
    ASSERT(it != fecList.end());

    FecBindVector::iterator dit = findFecEntry(fecDown, it->fecid, fromIP);
    ASSERT(dit == fecDown.end());

    // insert among received mappings

    fec_bind_t newItem;
    newItem.fecid = it->fecid;
    newItem.peer = fromIP;
    newItem.label = label;
    fecDown.push_back(newItem);

    // respond to pending requests

    PendingVector::iterator pit;
    for (pit = pending.begin(); pit != pending.end();)
    {
        if (pit->fecid != it->fecid)
        {
            pit++;
            continue;
        }

        EV << "there's pending request for this FEC from " << pit->peer << ", sending mapping" << endl;

        std::string inInterface = findInterfaceFromPeerAddr(pit->peer);
        std::string outInterface = findInterfaceFromPeerAddr(fromIP);
        LabelOpVector outLabel = LIBTable::swapLabel(label);

        fec_bind_t newItem;
        newItem.fecid = it->fecid;
        newItem.peer = pit->peer;
        newItem.label = lt->installLibEntry(-1, inInterface, outLabel, outInterface, LDP_USER_TRAFFIC);
        fecUp.push_back(newItem);

        EV << "installed LIB entry inLabel=" << newItem.label << " inInterface=" << inInterface <<
                " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        sendMapping(LABEL_MAPPING, pit->peer, newItem.label, it->addr, it->length);

        // remove request from the list
        pit = pending.erase(pit);
    }

    delete packet;
}

int LDP::findPeer(IPv4Address peerAddr)
{
    for (PeerVector::iterator i=myPeers.begin(); i!=myPeers.end(); ++i)
        if (i->peerIP==peerAddr)
            return i-myPeers.begin();
    return -1;
}

TCPSocket *LDP::findPeerSocket(IPv4Address peerAddr)
{
    // find peer in table and return its socket
    int i = findPeer(peerAddr);
    if (i==-1 || !(myPeers[i].socket) || myPeers[i].socket->getState()!=TCPSocket::CONNECTED)
        return NULL; // we don't have an LDP session to this peer
    return myPeers[i].socket;
}

TCPSocket *LDP::getPeerSocket(IPv4Address peerAddr)
{
    TCPSocket *sock = findPeerSocket(peerAddr);
    ASSERT(sock);
    if (!sock)
        error("No LDP session to peer %s yet", peerAddr.str().c_str());
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
    if (protocol == IP_PROT_UDP && check_and_cast<UDPPacket*>(ipdatagram->getEncapsulatedPacket())->getDestinationPort() == LDP_PORT)
        return false;

    // ...and session)
    if (protocol == IP_PROT_TCP && check_and_cast<TCPSegment*>(ipdatagram->getEncapsulatedPacket())->getDestPort() == LDP_PORT)
        return false;
    if (protocol == IP_PROT_TCP && check_and_cast<TCPSegment*>(ipdatagram->getEncapsulatedPacket())->getSrcPort() == LDP_PORT)
        return false;

    // regular traffic, classify, label etc.

    FecVector::iterator it;
    for (it = fecList.begin(); it != fecList.end(); it++)
    {
        if (!destAddr.prefixMatches(it->addr, it->length))
            continue;

        EV << "FEC matched: " << *it << endl;

        FecBindVector::iterator dit = findFecEntry(fecDown, it->fecid, it->nextHop);
        if (dit != fecDown.end())
        {
            outLabel = LIBTable::pushLabel(dit->label);
            outInterface = findInterfaceFromPeerAddr(it->nextHop);
            color = LDP_USER_TRAFFIC;
            EV << "mapping found, outLabel=" << outLabel << ", outInterface=" << outInterface << endl;
            return true;
        }
        else
        {
            EV << "no mapping for this FEC exists" << endl;
            return false;
        }
    }
    return false;
}

void LDP::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    ASSERT(category==NF_IPv4_ROUTE_ADDED || category==NF_IPv4_ROUTE_DELETED);

    EV << "routing table changed, rebuild list of known FEC" << endl;

    rebuildFecList();
}

void LDP::announceLinkChange(int tedlinkindex)
{
    TEDChangeInfo d;
    d.setTedLinkIndicesArraySize(1);
    d.setTedLinkIndices(0, tedlinkindex);
    nb->fireChangeNotification(NF_TED_CHANGED, &d);
}


