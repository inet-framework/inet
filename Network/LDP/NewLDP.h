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

#ifndef __NEWLDP_H__
#define __NEWLDP_H__


#include <string>
#include <omnetpp.h>
#include <iostream>
#include <vector>
#include "LDPPacket_m.h"
#include "MPLSAccess.h"
#include "LIBTableAccess.h"
#include "RoutingTableAccess.h"
#include "TCPSocketMap.h"


/**
 * LDP (rfc 3036) protocol implementation.
 */
class NewLDP: public cSimpleModule, public TCPSocket::CallbackInterface
{
  public:
    struct fec_src_bind
    {
        int fecId;
        IPAddress fec;  // actually the dest IP address
        string fromInterface;
    };

    struct peer_info
    {
        IPAddress peerIP;   // IP address of LDP peer
        bool activeRole;    // we're in active or passive role in this session
        TCPSocket *socket;  // TCP socket
        string linkInterface;
    };

  private:
    // configuration
    bool isIR;
    bool isER;
    double helloTimeout;  // FIXME obey

    //
    // state variables:
    //

    // the collection of all label requests pending on the current LSR
    // from upstream LSRs and itself.
    typedef vector<fec_src_bind> FecSenderBindVector;
    FecSenderBindVector fecSenderBinds;

    // the collection of all HELLO adjacencies.
    typedef vector<peer_info> PeerVector;
    PeerVector myPeers;

    //
    // other variables:
    //
    RoutingTableAccess routingTableAccess;
    LIBTableAccess libTableAccess;
    MPLSAccess mplsAccess;

    TCPSocket serverSocket;  // for listening on LDP_PORT
    TCPSocketMap socketMap;  // holds TCP connections with peers

    // hello timeout message
    cMessage *sendHelloMsg;

  private:
    /**
     * This method finds next peer in upstream direction
     */
    IPAddress locateNextHop(IPAddress dest);

    /**
     * This method maps the peerIP with the interface name in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    IPAddress findPeerAddrFromInterface(string interfaceName);

    //This method is the reserve of above method
    string findInterfaceFromPeerAddr(IPAddress peerIP);

    /** Utility: return peer's index in myPeers table, or -1 if not found */
    int findPeer(IPAddress peerAddr);

    /** Utility: return socket for given peer. Throws error if there's no TCP connection */
    TCPSocket *peerSocket(IPAddress peerAddr);

  public:
    Module_Class_Members(NewLDP,cSimpleModule, 0);

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void sendHelloTo(IPAddress dest);
    void openTCPConnectionToPeer(int peerIndex);

    void processLDPHello(LDPHello *msg);
    void processMessageFromTCP(cMessage *msg);
    void processRequestFromMPLSSwitch(cMessage *msg);
    void processLDPPacketFromTCP(LDPPacket *ldpPacket);

    void processLABEL_MAPPING(LDPLabelMapping *packet);
    void processLABEL_REQUEST(LDPLabelRequest *packet);

    /** @name TCPSocket::CallbackInterface callback methods */
    //@{
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketDataArrived(int connId, void *yourPtr, cMessage *msg, bool urgent);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    //@}
};

#endif


