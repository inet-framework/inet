/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#include <omnetpp.h>
#include <iostream>
#include <fstream>
#include "ConstType.h"
#include "NewLDP.h"
#include "LIBtable.h"
#include "MPLSModule.h"
#include "RoutingTable.h"
#include "UDPControlInfo_m.h"
#include "stlwatch.h"


#define LDP_PORT  646



Define_Module(NewLDP);


std::ostream& operator<<(std::ostream& os, const NewLDP::fec_src_bind& f)
{
    os << "fecId=" << f.fecId << "  fec=" << f.fec << "  fromInterface=" << f.fromInterface;
    return os;
}

std::ostream& operator<<(std::ostream& os, const NewLDP::peer_info& p)
{
    os << "peerIP=" << p.peerIP << "  interface=" << p.linkInterface <<
          "  activeRole=" << (p.activeRole ? "true" : "false") <<
          "  socket=" << (p.socket ? TCPSocket::stateName(p.socket->state()) : "NULL");
    return os;
}

void NewLDP::initialize()
{
    helloTimeout = par("helloTimeout").doubleValue();

    isIR = par("isIR"); // TBD we shouldn't need these params; see comments at isIR/isER usage in the code
    isER = par("isER");

    WATCH_VECTOR(myPeers);
    WATCH_VECTOR(fecSenderBinds);

    // schedule first hello
    sendHelloMsg = new cMessage("LDPSendHello");
    scheduleAt(exponential(0.1), sendHelloMsg); //FIXME

    // start listening for incoming conns
    ev << "Starting to listen on port " << LDP_PORT << " for incoming LDP sessions\n";
    serverSocket.setOutputGate(gate("to_tcp_interface"));
    serverSocket.bind(LDP_PORT);
    serverSocket.listen(true);
}

void NewLDP::handleMessage(cMessage *msg)
{
    if (msg==sendHelloMsg)
    {
        // every LDP capable router periodically sends HELLO messages to the
        // "all routers in the sub-network" multicast address
        ev << "Broadcasting LDP Hello\n";
        sendHelloTo(IPAddress("224.0.0.0"));

        // schedule next hello in 5 minutes
        scheduleAt(simTime()+300, sendHelloMsg);
    }
    else if (!strcmp(msg->arrivalGate()->name(), "from_udp_interface"))
    {
        // we can only receive LDP Hello from UDP (everything else goes over TCP)
        processLDPHello(check_and_cast<LDPHello *>(msg));
    }
    else if (!strcmp(msg->arrivalGate()->name(), "from_mpls_switch"))
    {
        processRequestFromMPLSSwitch(msg);
    }
    else if (!strcmp(msg->arrivalGate()->name(), "from_tcp_interface"))
    {
        processMessageFromTCP(msg);
    }
}

void NewLDP::sendHelloTo(IPAddress dest)
{
    RoutingTable *rt = routingTableAccess.get();

    LDPHello *hello = new LDPHello("LDP-Hello");
    hello->setType(HELLO);
    hello->setSenderAddress(rt->getRouterId());
    //hello->setReceiverAddress(...);
    //hello->setHoldTime(...);
    //hello->setRbit(...);
    //hello->setTbit(...);

    UDPControlInfo *controlInfo = new UDPControlInfo();
    //controlInfo->setSrcAddr(rt->getRouterId());
    controlInfo->setDestAddr(dest);
    controlInfo->setSrcPort(100);
    controlInfo->setDestPort(100);
    hello->setControlInfo(controlInfo);

    send(hello, "to_udp_interface");
}

void NewLDP::processLDPHello(LDPHello *msg)
{
    RoutingTable *rt = routingTableAccess.get();

    UDPControlInfo *controlInfo = check_and_cast<UDPControlInfo *>(msg->controlInfo());
    IPAddress peerAddr = controlInfo->getSrcAddr(); // FIXME use hello->senderAddress() instead?

    int inputPort = controlInfo->getInputPort();
    delete msg;

    ev << "Received LDP Hello from " << peerAddr << ", ";

    if (peerAddr.isNull() || peerAddr==rt->getRouterId())
    {
        // must be ourselves (we're also in the all-routers multicast group), ignore
        ev << "that's myself, ignore\n";
        return;
    }

    // peer already in table?
    int i = findPeer(peerAddr);
    if (i!=-1)
    {
        ev << "already in my peer table\n";
        return;
    }

    // not in table, add it
    peer_info info;
    info.peerIP = peerAddr;
    info.linkInterface = rt->interfaceByPortNo(inputPort)->name;
    info.activeRole = peerAddr.getInt() > rt->getRouterId().getInt();
    info.socket = NULL;
    myPeers.push_back(info);
    int peerIndex = myPeers.size()-1;

    ev << "added to peer table\n";
    ev << "We'll be " << (info.activeRole ? "ACTIVE" : "PASSIVE") << " in this session\n";

    // introduce ourselves with a Hello, then connect if we're in ACTIVE role
    sendHelloTo(peerAddr);
    if (info.activeRole)
    {
        ev << "Establishing session with it\n";
        openTCPConnectionToPeer(peerIndex);
    }
}

void NewLDP::openTCPConnectionToPeer(int peerIndex)
{
    TCPSocket *socket = new TCPSocket();
    socket->setOutputGate(gate("to_tcp_interface"));
    socket->setCallbackObject(this, (void*)peerIndex);
    socketMap.addSocket(socket);
    myPeers[peerIndex].socket = socket;

    socket->connect(myPeers[peerIndex].peerIP, LDP_PORT);
}

void NewLDP::processMessageFromTCP(cMessage *msg)
{
    TCPSocket *socket = socketMap.findSocketFor(msg);
    if (!socket)
    {
        // not yet in socketMap, must be new incoming connection.
        // find which peer it is and register connection
        socket = new TCPSocket(msg);
        socket->setOutputGate(gate("to_tcp_interface"));

        IPAddress peerAddr = socket->remoteAddress();

        int i = findPeer(peerAddr);
        if (i==-1 || myPeers[i].socket)
        {
            // nothing known about this guy, or already connected: refuse
            socket->close(); // FIXME: PEER_CLOSED and CLOSED notifications will come back to us, handle!!!
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

void NewLDP::socketEstablished(int, void *yourPtr)
{
    peer_info& peer = myPeers[(int)yourPtr];
    ev << "TCP connection established with peer " << peer.peerIP << "\n";

    // FIXME start LDP session setup (if we're on the active side?)
}

void NewLDP::socketDataArrived(int, void *yourPtr, cMessage *msg, bool)
{
    peer_info& peer = myPeers[(int)yourPtr];
    ev << "Message arrived over TCP from peer " << peer.peerIP << "\n";

    delete msg->removeControlInfo();
    processLDPPacketFromTCP(check_and_cast<LDPPacket *>(msg));
}

void NewLDP::socketPeerClosed(int, void *yourPtr)
{
    peer_info& peer = myPeers[(int)yourPtr];
    ev << "Peer " << peer.peerIP << " closed TCP connection\n";

/*
    // close the connection (if not already closed)
    if (socket.state()==TCPSocket::PEER_CLOSED)
    {
        ev << "remote TCP closed, closing here as well\n";
        close();
    }
*/
}

void NewLDP::socketClosed(int, void *yourPtr)
{
    peer_info& peer = myPeers[(int)yourPtr];
    ev << "TCP connection to peer " << peer.peerIP << " closed\n";

    // FIXME what now? reconnect after a delay?
}

void NewLDP::socketFailure(int, void *yourPtr, int code)
{
    peer_info& peer = myPeers[(int)yourPtr];
    ev << "TCP connection to peer " << peer.peerIP << " broken\n";

    // FIXME what now? reconnect after a delay?
}

void NewLDP::processRequestFromMPLSSwitch(cMessage *msg)
{
    // This is a request for new label finding
    RoutingTable *rt = routingTableAccess.get();

    int fecId = msg->par("fecId");
    IPAddress fecInt = IPAddress(msg->par("dest_addr").longValue());
    int gateIndex = msg->par("gateIndex");
    delete msg;

    InterfaceEntry *ientry = rt->interfaceByPortNo(gateIndex);
    string fromInterface = ientry->name;

    ev << "Request from MPLS for FEC=" << fecId << "  dest=" << fecInt <<
          "  inInterface=" << fromInterface << "\n";

    // LDP checks if there is any previous pending requests for
    // the same FEC.

    unsigned int i;
    for (i = 0; i < fecSenderBinds.size(); i++)
        if (fecSenderBinds[i].fec == fecInt)
            break;

    if (i != fecSenderBinds.size())
    {
        // there is a previous similar request
        ev << "Already issued Label Request for this FEC\n";
        return;
    }

    // LDP does the simple job of matching L3 routing to L2 routing

    // We need to find which peer (from L2 perspective) corresponds to
    // IP host of the next-hop.

    IPAddress nextPeerAddr = locateNextHop(fecInt);
    if (nextPeerAddr.isNull())
    {
        // bad luck
        ev << "No route found for this dest address\n";
        return;
    }

    // add to table
    fec_src_bind newBind;
    newBind.fec = fecInt;
    newBind.fromInterface = fromInterface;
    newBind.fecId = fecId;
    fecSenderBinds.push_back(newBind);

    // genarate new LABEL REQUEST and send downstream
    LDPLabelRequest *requestMsg = new LDPLabelRequest("Lb-Req");
    requestMsg->setType(LABEL_REQUEST);
    requestMsg->setFec(fecInt);  // FIXME this is actually the dest IP address!!!
    requestMsg->setFecId(fecId); // FIXME!!!

    requestMsg->setReceiverAddress(nextPeerAddr);
    requestMsg->setSenderAddress(rt->getRouterId());

    requestMsg->setLength(30*8); // FIXME find out actual length

    ev << "Request for FEC(" << fecInt << ") from outside\n";
    ev << "forward LABEL REQUEST to LSR(" << nextPeerAddr << ")\n";

    // send msg to peer over TCP
    peerSocket(nextPeerAddr)->send(requestMsg);
}


void NewLDP::processLDPPacketFromTCP(LDPPacket *ldpPacket)
{
    switch (ldpPacket->getType())
    {
    case HELLO:
        error("Received LDP HELLO over TCP (should arrive over UDP)");

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
        // processLABEL_WITHDRAW(ldpPacket);
        error("LDP PROC DEBUG: Received LDP LABEL_WITHDRAW message, unsupported in this version");
        break;

    case LABEL_RELEASE:
        // processLABEL_RELEASE(ldpPacket);
        error("LDP PROC DEBUG: Received LDP LABEL_RELEASE message, unsupported in this version");
        break;

    default:
        error("LDP PROC DEBUG: Unrecognized LDP Message Type, type is %d", ldpPacket->kind());
    }
}

IPAddress NewLDP::locateNextHop(IPAddress dest)
{
    // Mapping L3 IP-host of next hop to L2 peer address.
    RoutingTable *rt = routingTableAccess.get();

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
    //for (i=0; i < rt->numRoutingEntries(); i++)
    //    if (rt->routingEntry(i)->host == dest)
    //        break;
    //
    //if (i == rt->numRoutingEntries())
    //    return IPAddress();  // Signal an NOTIFICATION of NO ROUTE
    //
    int portNo = rt->outputPortNo(dest);
    if (portNo==-1)
        return IPAddress();  // no route

    string iName = rt->interfaceByPortNo(portNo)->name;
    return findPeerAddrFromInterface(iName);
}

// To allow this to work, make sure there are entries of hosts for all peers

IPAddress NewLDP::findPeerAddrFromInterface(string interfaceName)
{
    RoutingTable *rt = routingTableAccess.get();

    int i = 0;
    int k = 0;
    InterfaceEntry *interfacep = rt->interfaceByName(interfaceName.c_str());

    RoutingEntry *anEntry;

    for (i = 0; i < rt->numRoutingEntries(); i++)
    {
        for (k = 0; k < (int)myPeers.size(); k++)
        {
            anEntry = rt->routingEntry(i);
            if (anEntry->host==myPeers[k].peerIP && anEntry->interfacePtr==interfacep)
            {
                return myPeers[k].peerIP;
            }
            // addresses->push_back(peerIP[k]);
        }
    }

    // Return any IP which has default route - not in routing table entries
    for (i = 0; i < (int)myPeers.size(); i++)
    {
        for (k = 0; k < rt->numRoutingEntries(); k++)
        {
            anEntry = rt->routingEntry(i);
            if (anEntry->host == myPeers[i].peerIP)
                break;
        }
        if (k == rt->numRoutingEntries())
            break;
    }

    return myPeers[i].peerIP;

}

// Pre-condition: myPeers vector is finalized
string NewLDP::findInterfaceFromPeerAddr(IPAddress peerIP)
{
    RoutingTable *rt = routingTableAccess.get();
/*
    int i;
    for(int i=0;i<myPeers.size();i++)
    {
        if(myPeers[i].peerIP == peerIP)
            return string(myPeers[i].linkInterface);
    }
    return string("X");
*/
//    Rely on port index to find the interface name
    int portNo = rt->outputPortNo(peerIP);
    return rt->interfaceByPortNo(portNo)->name;

}

void NewLDP::processLABEL_REQUEST(LDPLabelRequest *packet)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();

    // Only accept new requests; discard if duplicate
    IPAddress fec = packet->getFec();
    IPAddress srcAddr = packet->getSenderAddress();
    int fecId = packet->getFecId();

    ev << "Label Request from LSR " << srcAddr << " for FEC " << fec << "\n";

    unsigned int i;
    for (i=0; i < fecSenderBinds.size(); i++)
        if (fecSenderBinds[i].fec == fec)
            break;
    if (i != fecSenderBinds.size())
    {
        // repeated request: do nothing (FIXME is this OK?)
        ev << "Repeated request, ignoring\n";
        delete packet;
        return;
    }

    // This is the incoming interface if label found
    string inInterface = findInterfaceFromPeerAddr(srcAddr);

    // Add new request to table
    fec_src_bind newBind;
    newBind.fec = fec;
    newBind.fromInterface = inInterface;
    fecSenderBinds.push_back(newBind);

    // Look up FEC in our LIB table
    int inLabel = lt->findInLabel(fec.getInt());

    if (inLabel!=-1)
    {
        // We already have a mapping for this FEC, let our upstream peer know
        // about it by sending back a Label Mapping message
        ev << "FEC " << fec << " found in LIB: inLabel=" << inLabel << "\n";
        ev << "Sending back a Label Mapping message about it\n";

        // Send LABEL MAPPING upstream
        LDPLabelMapping *lmMessage = new LDPLabelMapping("Lb-Mapping");
        lmMessage->setType(LABEL_MAPPING);
        lmMessage->setLength(30*8); // FIXME find out actual length
        lmMessage->setReceiverAddress(srcAddr);
        lmMessage->setSenderAddress(rt->getRouterId());
        lmMessage->setLabel(inLabel);
        lmMessage->setFec(fec);
        lmMessage->setFecId(fecId);

        // send msg to peer over TCP
        peerSocket(srcAddr)->send(lmMessage);
        delete packet;
    }
    else if (isER) // FIXME this condition is not convincing. Rather, we should check if we're
                   // ER for *this* FEC; that is, next hop is NOT an LSR (not in our peer table)
    {
        // Note this is the ER router, we must base on rt to find the next hop
        // Rely on port index to find the to-outside interface name
        int portNo = rt->outputPortNo(fec);
        if (portNo==-1)
            error("FEC %s is unroutable", fec.str().c_str());
        string outInterface = string(rt->interfaceByPortNo(portNo)->name);

        int inLabel = lt->installNewLabel(-1, inInterface, outInterface, fecId, POP_OPER);

        ev << "Egress router reached. Assigned inLabel=" << inLabel << " to FEC " << fec <<
              ", outInterface=" << outInterface << "\n";
        ev << "Sending back a Label Mapping message about it\n";

        // Send LABEL MAPPING upstream
        LDPLabelMapping *lmMessage = new LDPLabelMapping("Lb-Mapping");
        lmMessage->setType(LABEL_MAPPING);
        lmMessage->setLength(30*8); // FIXME find out actual length
        lmMessage->setReceiverAddress(srcAddr);
        lmMessage->setSenderAddress(rt->getRouterId());
        lmMessage->setLabel(inLabel);
        lmMessage->setFec(fec);
        lmMessage->setFecId(fecId);

        // send msg to peer over TCP
        peerSocket(srcAddr)->send(lmMessage);
        delete packet;
    }
    else  // Propagate downstream
    {
        ev << "FEC " << fec << " not in our LIB, propagating Label Request downstream\n";

        // Set parameters allowed to send downstream
        IPAddress peerIP = locateNextHop(fec);
        if (peerIP.isNull())
            opp_error("Cannot find downstream neighbor for FEC %s", fec.str().c_str());

        packet->setReceiverAddress(peerIP);
        packet->setSenderAddress(rt->getRouterId());
        peerSocket(peerIP)->send(packet);
    }
}

void NewLDP::processLABEL_MAPPING(LDPLabelMapping * packet)
{
    LIBTable *lt = libTableAccess.get();
    MPLSModule *mplsMod = mplsAccess.get();

    IPAddress fec = packet->getFec();
    int label = packet->getLabel();
    IPAddress fromIP = packet->getSenderAddress();
    int fecId = packet->getFecId(); // FIXME was not used (?)

    ev << "Gets mapping Label =" << label << " for fec =" <<
        IPAddress(fec) << " from LSR(" << IPAddress(fromIP) << ")\n";

    // This is the outgoing interface for the FEC
    string outInterface = findInterfaceFromPeerAddr(fromIP);
    string inInterface;

    if (isIR)  // FIXME rather, we should check if we're IR for *this* FEC, that is, we haven't received Label Request from upstream peer for this FEC
    {
        cMessage *signalMPLS = new cMessage("path created");

        signalMPLS->addPar("label") = label;
        signalMPLS->addPar("fecInt") = fec.getInt();
        signalMPLS->addPar("my_name") = 0;
        int myfecId = -1;

        // Install new label
        for (unsigned int k = 0; k < fecSenderBinds.size(); k++)
        {
            if (fecSenderBinds[k].fec == fec)
            {
                myfecId = fecSenderBinds[k].fecId;
                signalMPLS->addPar("fecId") = myfecId;
                inInterface = fecSenderBinds[k].fromInterface;
                // Remove the item
                fecSenderBinds.erase(fecSenderBinds.begin() + k);

                break;
            }
        }

        lt->installNewLabel(label, inInterface, outInterface, myfecId, PUSH_OPER);

        ev << "Send to my MPLS module\n";
        sendDirect(signalMPLS, 0.0, mplsMod, "fromSignalModule");
        delete packet;
    }
    else
    {
        // Install new label
        for (unsigned int k = 0; k < fecSenderBinds.size(); k++)
        {
            if (fecSenderBinds[k].fec == fec)
            {
                inInterface = fecSenderBinds[k].fromInterface;
                // Remove the item
                fecSenderBinds.erase(fecSenderBinds.begin() + k);
                break;
            }
        }

        // Install new label
        int inLabel = lt->installNewLabel(label, inInterface, outInterface, fec.getInt(), SWAP_OPER);
        packet->setLabel(inLabel);
        packet->setFec(fec);
        IPAddress addrToSend = findPeerAddrFromInterface(inInterface);

        RoutingTable *rt = routingTableAccess.get();

        packet->setReceiverAddress(addrToSend);
        packet->setSenderAddress(rt->getRouterId());

        ev << "Sends Label mapping label=" << inLabel <<
            " for fec =" << IPAddress(fec) << " to " << "LSR(" << addrToSend << ")\n";

        peerSocket(addrToSend)->send(packet);
    }
}

int NewLDP::findPeer(IPAddress peerAddr)
{
    for (PeerVector::iterator i=myPeers.begin(); i!=myPeers.end(); ++i)
        if (i->peerIP==peerAddr)
            return i-myPeers.begin();
    return -1;
}

TCPSocket *NewLDP::peerSocket(IPAddress peerAddr)
{
    // find peer in table and return its socket
    int i = findPeer(peerAddr);
    if (i==-1 || !(myPeers[i].socket) || myPeers[i].socket->state()!=TCPSocket::CONNECTED)
    {
        // we don't have an LDP session to this peer
        error("No LDP session to peer %s yet", peerAddr.str().c_str());
    }
    return myPeers[i].socket;
}
