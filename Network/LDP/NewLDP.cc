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
#include "LIBTable.h"
#include "MPLSModule.h"
#include "RoutingTable.h"
#include "UDPControlInfo_m.h"


#define LDP_PORT  646



Define_Module(NewLDP);


void NewLDP::initialize()
{
    helloTimeout = par("helloTimeout").doubleValue();

    // Find its own address, this is important since it needs to notify all neigbour
    // this information in HELLO message.
    cModule *curmod = this;
    for (curmod = parentModule(); curmod != NULL; curmod = curmod->parentModule())
    {
        if (curmod->hasPar("local_addr"))  // FIXME!!!!
        {
            local_addr = curmod->par("local_addr").stringValue();
            isIR = curmod->par("isIR");
            isER = curmod->par("isER");
            break;
        }
    }
    if (local_addr.isNull())
        error("Cannot find local_address");

    // schedule first hello
    sendHelloMsg = new cMessage("LDPSendHello");
    scheduleAt(1, sendHelloMsg);

    // start listening for incoming conns
    ev << "Starting to listen on port " << LDP_PORT << " for incoming LDP sessions\n";
    serverSocket.setOutputGate(gate("to_tcp_interface"));
    serverSocket.bind(local_addr, LDP_PORT);
    serverSocket.listen(true);
}

void NewLDP::handleMessage(cMessage *msg)
{
    if (msg==sendHelloMsg)
    {
        // periodically send out LDP Hellos
        broadcastHello();
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

void NewLDP::broadcastHello()
{
    // Each LDP capable router sends HELLO messages to a multicast address to all
    // of routers in the sub-network

    ev << "Broadcasting LDP Hello\n";

    LDPHello *hello = new LDPHello("ldp-hello");
    //hello->setHoldTime(...);
    //hello->setRbit(...);
    //hello->setTbit(...);

    UDPControlInfo *controlInfo = new UDPControlInfo();
    controlInfo->setSrcAddr(local_addr);
    controlInfo->setDestAddr(IPAddress("224.0.0.0"));
    controlInfo->setSrcPort(100);
    controlInfo->setDestPort(100);
    hello->setControlInfo(controlInfo);

    send(hello, "to_udp_interface");

    // schedule next hello in 5 minutes
    scheduleAt(simTime()+300, sendHelloMsg);
}

void NewLDP::processLDPHello(LDPHello *msg)
{
    UDPControlInfo *controlInfo = check_and_cast<UDPControlInfo *>(msg->removeControlInfo());
    IPAddress peerAddr = controlInfo->getSrcAddr();
    delete msg;
    delete controlInfo;

    ev << "Received LDP Hello from " << peerAddr << ", ";

    if (peerAddr.isNull() || peerAddr==local_addr)
    {
        // must be ourselves (we're also in the all-routers multicast group), ignore
        ev << "ignore\n";
        return;
    }

    string inInterface = this->findInterfaceFromPeerAddr(peerAddr);

    // peer already in table?
    PeerVector::iterator i;
    for (i=myPeers.begin(); i!=myPeers.end(); ++i)
        if (i->peerIP==peerAddr)
            break;
    if (i!=myPeers.end())
    {
        ev << "already in my peer table\n";
        return;
    }

    // not in table, add it
    peer_info info;
    info.peerIP = peerAddr;
    info.linkInterface = inInterface;
    info.activeRole = peerAddr.getInt() > local_addr.getInt();
    info.connected = false;
    myPeers.push_back(info);
    int peerIndex = myPeers.size()-1;

    ev << "added to peer table\n";
    ev << "We'll be " << (info.activeRole ? "ACTIVE" : "PASSIVE") << " in this session\n";

    // initiate connection to peer
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

        PeerVector::iterator i;
        for (i=myPeers.begin(); i!=myPeers.end(); ++i)
            if (i->peerIP==peerAddr)
                break;
        if (i==myPeers.end() || (i!=myPeers.end() && i->connected))
        {
            // nothing known about this guy, or already connected: refuse
            socket->close(); // FIXME should rather be connection reset!
            delete socket;
            delete msg;
            return;
        }
        i->connected = true;
        int peerIndex = i - myPeers.begin();

        socket->setOutputGate(gate("to_tcp_interface"));
        socket->setCallbackObject(this, (void *)peerIndex);
        socketMap.addSocket(socket);
    }

    // dispatch to socketEstablished(), socketDataArrived(), socketPeerClosed()
    // or socketFailure()
    socket->processMessage(msg);
}

void NewLDP::socketEstablished(int, void *yourPtr)
{
    peer_info& peer = myPeers[(int)yourPtr];
    peer.connected = true;
    ev << "TCP connection established with peer " << peer.peerIP << "\n";

    // FIXME start LDP session setup (if we're on the active side?)
}

void NewLDP::socketDataArrived(int, void *yourPtr, cMessage *msg, bool)
{
    peer_info& peer = myPeers[(int)yourPtr];
    ev << "Message arrived over TCP from peer " << peer.peerIP << "\n";

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

    int fecId = msg->par("FEC");
    int fecInt = msg->par("dest_addr");
    // int dest =msg->par("dest_addr"); FIXME was not used (?)
    int gateIndex = msg->par("gateIndex");
    InterfaceEntry *ientry = rt->interfaceByPortNo(gateIndex);

    string fromInterface = string(ientry->name.c_str());

    // LDP checks if there is any previous pending requests for
    // the same FEC.

    int i;
    for (i = 0; i < fecSenderBinds.size(); i++)
    {
        if (fecSenderBinds[i].fec == fecInt)
            break;
    }

    if (i == fecSenderBinds.size())  // There is no previous same requests
    {

        fec_src_bind newBind;
        newBind.fec = fecInt;
        newBind.fromInterface = fromInterface;
        newBind.fecID = fecId;

        fecSenderBinds.push_back(newBind);

        // Genarate new LABEL REQUEST and send downstream

        LDPLabelRequest *requestMsg = new LDPLabelRequest();
        requestMsg->setFec(fecInt);
        requestMsg->addPar("fecId") = fecId;

        // LDP does the simple job of matching L3 routing to L2 routing

        // We need to find which peer (from L2 perspective) corresponds to
        // IP host of the next-hop.

        IPAddress nextPeerAddr = locateNextHop(fecInt);

        if (!nextPeerAddr.isNull())
        {
            requestMsg->setReceiverAddress(nextPeerAddr);
            requestMsg->setSenderAddress(local_addr);

            ev << "LSR(" << local_addr <<
                "):Request for FEC(" << fecInt << ") from outside \n";

            ev << "LSR(" << local_addr <<
                ")  forward LABEL REQUEST to " <<
                "LSR(" << nextPeerAddr << ")\n";

            delete msg;

            send(requestMsg, "to_tcp_interface");
        }
        else
        {
            // Send a NOTIFICATION of NO ROUTE message
            ev << "LSR(" << local_addr <<
                "): NO ROUTE found for FEC(" << fecInt << "\n";

            delete msg;
        }
    }
}


void NewLDP::processLDPPacketFromTCP(LDPPacket *ldpPacket)
{
    switch (ldpPacket->kind())
    {
    case HELLO:
        // processHELLO(ldpPacket);
        ev << "Received LDP HELLO message\n";  // FIXME so what to do with it? --Andras
        delete ldpPacket;
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

IPAddress NewLDP::locateNextHop(int fec)
{
    // Mapping L3 IP-host of next hop  to L2 peer address.
    RoutingTable *rt = routingTableAccess.get();

    // Lookup the routing table, rfc3036
    // When the FEC for which a label is requested is a Prefix FEC Element or
    // a Host Address FEC Element, the receiving LSR uses its routing table to determine
    // its response. Unless its routing table includes an entry that exactly matches
    // the requested Prefix or Host Address, the LSR must respond with a
    // No Route Notification message.
    int i;
    for (i=0; i < rt->numRoutingEntries(); i++)
        if (rt->routingEntry(i)->host.getInt() == fec)
            break;

    if (i == rt->numRoutingEntries())
        return IPAddress();  // Signal an NOTIFICATION of NO ROUTE

    // Find out the IP of the other end LSR
    string iName = string(rt->routingEntry(i)->interfaceName.c_str());

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
        for (k = 0; k < myPeers.size(); k++)
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
    for (i = 0; i < myPeers.size(); i++)
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
    return string(rt->interfaceByPortNo(portNo)->name.c_str());

}

void NewLDP::processLABEL_REQUEST(LDPLabelRequest *packet)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();

    // Only accept new requests
    int fec = packet->getFec();
    IPAddress srcAddr = packet->getSenderAddress();
    int fecId = packet->par("fecId");

    // This is the incoming interface if label found
    string fromInterface = findInterfaceFromPeerAddr(srcAddr);

    // This is the outgoing interface if label found
    string nextInterface = findInterfaceFromPeerAddr(IPAddress(fec));  //FIXME what the holy shit???? --Andras

    int i;
    for (i = 0; i < fecSenderBinds.size(); i++)
    {
        if (fecSenderBinds[i].fec == fec)
            break;
    }

    if (i == fecSenderBinds.size())
    {
        // New request
        fec_src_bind newBind;
        newBind.fec = fec;
        newBind.fromInterface = fromInterface;
        fecSenderBinds.push_back(newBind);
    }
    else
        return;  // Do nothing it is repeated request

    // Look up table for this fec
    int label;
    string outgoingInterface;
    bool found = lt->resolveFec(fec, label, outgoingInterface);

    ev << "Request from LSR(" << IPAddress(srcAddr) << ") for fec=" << IPAddress(fec) << ")\n";

    if (found)  // Found the label
    {
        ev << "LSR(" << local_addr <<
            "): Label =" << label << " found for fec =" << IPAddress(fec) << "\n";

        // Construct a label mapping message

        LDPLabelMapping *lmMessage = new LDPLabelMapping();

        lmMessage->setLabel(label);
        lmMessage->setFec(fec);

        // Set dest to the requested upstream LSR
        lmMessage->setReceiverAddress(srcAddr);
        lmMessage->setSenderAddress(local_addr);
        lmMessage->addPar("fecId") = fecId;

        ev << "LSR(" << local_addr <<
            "): Send Label mapping(fec=" << IPAddress(fec) << ",label=" << label << ")to " <<
            "LSR(" << IPAddress(srcAddr) << ")\n";

        send(lmMessage, "to_tcp_interface");

        delete packet;

    }
    else if (isER)
    {
        ev << "LSR(" << local_addr <<
            "): Generates new label for the fec " << IPAddress(fec) << "\n";

        // Install new labels
        // Note this is the ER router, we must base on rt to find the next hop
        // Rely on port index to find the to-outside interface name
        // int index = rt->outputPortNo(IPAddress(peerIP));
        // nextInterface= string(rt->interfaceByPortNo(index)->name);
        int inLabel = (lt->installNewLabel(-1, fromInterface, nextInterface, fecId, POP_OPER)); // fec));

        // Send LABEL MAPPING upstream

        LDPLabelMapping *lmMessage = new LDPLabelMapping();

        lmMessage->setLabel(inLabel);
        lmMessage->setFec(fec);

        // Set dest to the requested upstream LSR
        lmMessage->setReceiverAddress(srcAddr);
        lmMessage->setSenderAddress(local_addr);
        lmMessage->addPar("fecId") = fecId;

        ev << "Send Label mapping to " << "LSR(" << srcAddr << ")\n";

        send(lmMessage, "to_tcp_interface");

        delete packet;
    }
    else  // Propagate downstream
    {
        ev << "Cannot find label for the fec " << IPAddress(fec) << "\n"; // FIXME what???

        // Set paramters allowed to send downstream

        IPAddress peerIP = locateNextHop(fec);

        if (!peerIP.isNull())
        {
            packet->setReceiverAddress(peerIP);
            packet->setSenderAddress(local_addr);

            ev << "Propagating Label Request from LSR(" <<
                packet->getSenderAddress() << " to " <<
                packet->getReceiverAddress() << ")\n";

            send(packet, "to_tcp_interface");
        }
    }
}

void NewLDP::processLABEL_MAPPING(LDPLabelMapping * packet)
{
    LIBTable *lt = libTableAccess.get();
    MPLSModule *mplsMod = mplsAccess.get();

    int fec = packet->getFec();
    int label = packet->getLabel();
    IPAddress fromIP = packet->getSenderAddress();
    // int fecId = packet->par("fecId"); -- FIXME was not used (?)

    ev << "LSR(" << local_addr << ") gets mapping Label =" << label << " for fec =" <<
        IPAddress(fec) << " from LSR(" << IPAddress(fromIP) << ")\n";

    // This is the outgoing interface for the FEC
    string outInterface = findInterfaceFromPeerAddr(fromIP);
    string inInterface;

    if (isIR)
    {
        cMessage *signalMPLS = new cMessage();

        signalMPLS->addPar("label") = label;

        signalMPLS->addPar("fecInt") = fec;
        signalMPLS->addPar("my_name") = 0;
        int myfecID = -1;

        // Install new label
        for (int k = 0; k < fecSenderBinds.size(); k++)
        {
            if (fecSenderBinds[k].fec == fec)
            {
                myfecID = fecSenderBinds[k].fecID;
                signalMPLS->addPar("fec") = myfecID;
                inInterface = fecSenderBinds[k].fromInterface;
                // Remove the item
                fecSenderBinds.erase(fecSenderBinds.begin() + k);

                break;
            }
        }

        lt->installNewLabel(label, inInterface, outInterface, myfecID, PUSH_OPER);

        ev << "Send to my MPLS module\n";
        sendDirect(signalMPLS, 0.0, mplsMod, "fromSignalModule");
        delete packet;
    }
    else
    {
        // Install new label
        for (int k = 0; k < fecSenderBinds.size(); k++)
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
        int inLabel = lt->installNewLabel(label, inInterface, outInterface, fec, SWAP_OPER);
        packet->setLabel(inLabel);
        packet->setFec(fec);
        IPAddress addrToSend = findPeerAddrFromInterface(inInterface);

        packet->setReceiverAddress(addrToSend);
        packet->setSenderAddress(local_addr);

        ev << "LSR(" << local_addr << ") sends Label mapping label=" << inLabel <<
            " for fec =" << IPAddress(fec) << " to " << "LSR(" << addrToSend << ")\n";

        send(packet, "to_tcp_interface");

        // delete packet;
    }
}


