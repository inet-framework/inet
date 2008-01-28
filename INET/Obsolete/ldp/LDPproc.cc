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
#include "LDPpacket.h"
#include "tcp.h"
#include "LDPproc.h"
#include "RoutingTable.h"
#include "MPLSModule.h"

Define_Module(LDPproc);

void LDPproc::initialize()
{
    // Get number of peers, we don't want to rely on intialisation time period only
    // This is an assumption
    peerNo = (int) par("peerNo").longValue();
    helloTimeout = par("helloTimeout").doubleValue();
}

void LDPproc::activity()
{

/********************************************************************************
*                                                                               *
*                                PARAMETERS INTIALISED                          *
*                                                                               *
*********************************************************************************/

    // Find its own address, this is important since it needs to notify all neigbour
    // this information in HELLO message.

    cModule *curmod = this;
    for (curmod = parentModule(); curmod != NULL; curmod = curmod->parentModule())
    {
        if (curmod->hasPar("local_addr"))  // FIXME!!!!
        {
            local_addr = IPAddress(curmod->par("local_addr").stringValue()).getInt();
            id = string(curmod->par("id").stringValue());
            isIR = curmod->par("isIR");
            isER = curmod->par("isER");
            break;
        }
    }

    ev << "*************" << id.c_str() << " LDP daemon starts*************\n";

    if (local_addr == -1)
        ev << "Warning - Cannot find my address - Set to 0\n";

/********************************************************************************
*                                                                               *
*                                PHASE 1: DISCOVERY                             *
*                                                                               *
*********************************************************************************/

    // Discovery - signal UDP

    // Each LDP capable router sends HELLO messages to a multicast address to all
    // of routers in the sub-network

    cMessage *helloMsg = new cMessage("ldp-broadcast-request");

    helloMsg->setKind(LDP_BROADCAST_REQUEST);
    helloMsg->addPar("peerID") = id.c_str();

    send(helloMsg, "to_udp_interface");

    // Wait for all replies from peers - MESSAGES from UDP interface
    int i;
    for (i = 0; i < peerNo; i++)
    {
        cMessage *msg = receive();
        if (!strcmp(msg->arrivalGate()->name(), "from_udp_interface"))
        {
            int anAddr = IPAddress((msg->par("src_addr").stringValue())).getInt();
            string anID = string(msg->par("peerID").stringValue());
            string anInterface = this->findInterfaceFromPeerAddr(anAddr);

            ev << "LSR(" << IPAddress(local_addr) << "): " <<
                "get multicast from " << IPAddress(anAddr) << "\n";

            peer_info info;

            info.peerIP = anAddr;
            info.linkInterface = anInterface;
            info.peerID = anID;

            if (anAddr > local_addr)
                info.role = string("Client");
            else
                info.role = string("Server");

            // Initialize the table of Hello adjacencies.

            myPeers.push_back(info);
        }
        else
        {
            ev << "Received non-UDP messages during initialisation, deleting\n";
            delete msg;
        }
    }

/********************************************************************************
*                                                                                *
*                                PHASE 2: LDP INIT                                  *
*                                                                                *
*********************************************************************************/

    // Problems:  Still looking for away to yeal running priority to other modules. Does
    // Omnet++ allows any ways to do this ?

    // PHASE 2: Initialisation - Establish LDP/TCP connections

    for (i = 0; i < myPeers.size(); i++)
    {
        int destIP = myPeers[i].peerIP;
        // signal TCP
        if (destIP > local_addr)
        {
            cMessage *kickOffTCP = new cMessage();
            kickOffTCP->setKind(LDP_CLIENT_CREATE);
            kickOffTCP->addPar("peerIP") = destIP;
            // wait(1); // Smooth the sending
            send(kickOffTCP, "to_tcp_interface");
        }
        ev << "LSR(" << IPAddress(local_addr) << "): " <<
            "opens TCP connection to LSR(" << IPAddress(destIP) << ")\n";
    }

    // Signal the MPLS that it can start making queries
    ev << "Finished LDP session setup with peers\n";
    MPLSModule *mplsMod = mplsAccess.get();
    cMessage *signalMsg = new cMessage();
    sendDirect(signalMsg, 0.0, mplsMod, "fromSignalModule");

/********************************************************************************
*                                                                                *
*                                PHASE 3: LDP OPERATION                          *
*                                                                                *
*********************************************************************************/

    // Operation - Exchange and process LDP packets

    // Expect outer host sends packets after the system has been initialised

    for (;;)
    {
        cMessage *msg = receive();

        if (!strcmp(msg->arrivalGate()->name(), "from_mpls_switch"))
        {
            processRequestFromMPLSSwitch(msg);
        }
        else if (!strcmp(msg->arrivalGate()->name(), "from_tcp_interface"))
        {
            processLDPPacketFromTCP(check_and_cast<LDPPacket *>(msg));
        }
    }

}


void LDPproc::processRequestFromMPLSSwitch(cMessage *msg)
{
    // This is a request for new label finding
    RoutingTable *rt = routingTableAccess.get();

    int fecId = msg->par("FEC");
    int fecInt = msg->par("dest_addr");
    // int dest =msg->par("dest_addr"); FIXME was not used (?)
    int gateIndex = msg->par("gateIndex");
    InterfaceEntry *ientry = rt->interfaceByPortNo(gateIndex);

    string fromInterface = ientry->name;

    // LDP checks if there is any previous pending requests for
    // the same FEC.

    int i;
    for (i = 0; i < FecSenderBinds.size(); i++)
    {
        if (FecSenderBinds[i].fec == fecInt)
            break;
    }

    if (i == FecSenderBinds.size())  // There is no previous same requests
    {

        fec_src_bind newBind;
        newBind.fec = fecInt;
        newBind.fromInterface = fromInterface;
        newBind.fecID = fecId;

        FecSenderBinds.push_back(newBind);

        // Genarate new LABEL REQUEST and send downstream

        LDPLabelRequest *requestMsg = new LDPLabelRequest();
        requestMsg->setFec(fecInt);
        requestMsg->addPar("fecId") = fecId;

        // LDP does the simple job of matching L3 routing to L2 routing

        // We need to find which peer (from L2 perspective) corresponds to
        // IP host of the next-hop.

        int nextPeerAddr = locateNextHop(fecInt);

        if (nextPeerAddr != 0)
        {
            requestMsg->setReceiverAddress(nextPeerAddr);
            requestMsg->setSenderAddress(local_addr);

            ev << "LSR(" << IPAddress(local_addr) <<
                "):Request for FEC(" << fecInt << ") from outside \n";

            ev << "LSR(" << IPAddress(local_addr) <<
                ")  forward LABEL REQUEST to " <<
                "LSR(" << IPAddress(nextPeerAddr) << ")\n";

            delete msg;

            send(requestMsg, "to_tcp_interface");
        }
        else
        {
            // Send a NOTIFICATION of NO ROUTE message
            ev << "LSR(" << IPAddress(local_addr) <<
                "): NO ROUTE found for FEC(" << fecInt << "\n";

            delete msg;
        }
    }
}


void LDPproc::processLDPPacketFromTCP(LDPPacket *ldpPacket)
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

int LDPproc::locateNextHop(int fec)
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
        return 0;  // Signal an NOTIFICATION of NO ROUTE

    // Find out the IP of the other end LSR
    string iName = string(rt->routingEntry(i)->interfaceName.c_str());

    return findPeerAddrFromInterface(iName);

}

// To allow this to work, make sure there are entries of hosts for all peers

int LDPproc::findPeerAddrFromInterface(string interfaceName)
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
            if (anEntry->host.getInt()==myPeers[k].peerIP && anEntry->interfacePtr==interfacep)
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
            if ((anEntry->host.getInt()) == (myPeers[i].peerIP))
                break;
        }
        if (k == rt->numRoutingEntries())
            break;
    }

    return myPeers[i].peerIP;

}

// Pre-condition: myPeers vector is finalized
string LDPproc::findInterfaceFromPeerAddr(int peerIP)
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
    int portNo = rt->outputPortNo(IPAddress(peerIP));
    return string(rt->interfaceByPortNo(portNo)->name.c_str());

}

void LDPproc::processLABEL_REQUEST(LDPLabelRequest * packet)
{
    RoutingTable *rt = routingTableAccess.get();
    LIBTable *lt = libTableAccess.get();

    // Only accept new requests
    int fec = packet->getFec();
    int srcAddr = packet->getSenderAddress();
    int fecId = packet->par("fecId");

    // This is the incoming interface if label found
    string fromInterface = findInterfaceFromPeerAddr(srcAddr);

    // This is the outgoing interface if label found
    string nextInterface = findInterfaceFromPeerAddr(fec);

    int i;
    for (i = 0; i < FecSenderBinds.size(); i++)
    {
        if (FecSenderBinds[i].fec == fec)
            break;
    }

    if (i == FecSenderBinds.size())
    {
        // New request
        fec_src_bind newBind;
        newBind.fec = fec;
        newBind.fromInterface = fromInterface;
        FecSenderBinds.push_back(newBind);
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
        ev << "LSR(" << IPAddress(local_addr) <<
            "): Label =" << label << " found for fec =" << IPAddress(fec) << "\n";

        // Construct a label mapping message

        LDPLabelMapping *lmMessage = new LDPLabelMapping();

        lmMessage->setMapping(label, fec);

        // Set dest to the requested upstream LSR
        lmMessage->setReceiverAddress(srcAddr);
        lmMessage->setSenderAddress(local_addr);
        lmMessage->addPar("fecId") = fecId;

        ev << "LSR(" << IPAddress(local_addr) <<
            "): Send Label mapping(fec=" << IPAddress(fec) << ",label=" << label << ")to " <<
            "LSR(" << IPAddress(srcAddr) << ")\n";

        send(lmMessage, "to_tcp_interface");

        delete packet;

    }
    else if (isER)
    {
        ev << "LSR(" << IPAddress(local_addr) <<
            "): Generates new label for the fec " << IPAddress(fec) << "\n";

        // Install new labels
        // Note this is the ER router, we must base on rt to find the next hop
        // Rely on port index to find the to-outside interface name
        // int index = rt->outputPortNo(IPAddress(peerIP));
        // nextInterface= string(rt->interfaceByPortNo(index)->name);
        int inLabel = (lt->installNewLabel(-1, fromInterface, nextInterface, fecId, POP_OPER)); // fec));

        // Send LABEL MAPPING upstream

        LDPLabelMapping *lmMessage = new LDPLabelMapping();

        lmMessage->setMapping(inLabel, fec);

        // Set dest to the requested upstream LSR
        lmMessage->setReceiverAddress(srcAddr);
        lmMessage->setSenderAddress(local_addr);
        lmMessage->addPar("fecId") = fecId;

        ev << "Send Label mapping to " << "LSR(" << IPAddress(srcAddr) << ")\n";

        send(lmMessage, "to_tcp_interface");

        delete packet;
    }
    else  // Propagate downstream
    {
        ev << "Cannot find label for the fec " << IPAddress(fec) << "\n";

        // Set paramters allowed to send downstream

        int peerIP = locateNextHop(fec);

        if (peerIP != 0)
        {
            packet->setReceiverAddress(peerIP);
            packet->setSenderAddress(local_addr);

            ev << "Propagating Label Request from LSR(" <<
                IPAddress(packet->getSenderAddress()) << " to " <<
                IPAddress(packet->getReceiverAddress()) << ")\n";

            send(packet, "to_tcp_interface");
        }
    }
}

void LDPproc::processLABEL_MAPPING(LDPLabelMapping * packet)
{
    LIBTable *lt = libTableAccess.get();
    MPLSModule *mplsMod = mplsAccess.get();

    int fec = packet->getFec();
    int label = packet->getLabel();
    int fromIP = packet->getSenderAddress();
    // int fecId = packet->par("fecId"); -- FIXME was not used (?)

    ev << "LSR(" << IPAddress(local_addr) << ") gets mapping Label =" << label << " for fec =" <<
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
        for (int k = 0; k < FecSenderBinds.size(); k++)
        {
            if (FecSenderBinds[k].fec == fec)
            {
                myfecID = FecSenderBinds[k].fecID;
                signalMPLS->addPar("fec") = myfecID;
                inInterface = FecSenderBinds[k].fromInterface;
                // Remove the item
                FecSenderBinds.erase(FecSenderBinds.begin() + k);

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
        for (int k = 0; k < FecSenderBinds.size(); k++)
        {
            if (FecSenderBinds[k].fec == fec)
            {
                inInterface = FecSenderBinds[k].fromInterface;
                // Remove the item
                FecSenderBinds.erase(FecSenderBinds.begin() + k);
                break;
            }
        }

        // Install new label
        int inLabel = lt->installNewLabel(label, inInterface, outInterface, fec, SWAP_OPER);
        packet->setMapping(inLabel, fec);
        int addrToSend = this->findPeerAddrFromInterface(inInterface);

        packet->setReceiverAddress(addrToSend);
        packet->setSenderAddress(local_addr);

        ev << "LSR(" << IPAddress(local_addr) << ") sends Label mapping label=" << inLabel <<
            " for fec =" << IPAddress(fec) << " to " << "LSR(" << IPAddress(addrToSend) << ")\n";

        send(packet, "to_tcp_interface");

        // delete packet;
    }
}
