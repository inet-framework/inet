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
  private:
    struct fec_src_bind
    {
        int fec;
        string fromInterface;
        int fecID;
    };

    struct peer_info
    {
        IPAddress peerIP;
        string role;
        string linkInterface;
    };

  private:
    RoutingTableAccess routingTableAccess;
    LIBTableAccess libTableAccess;
    MPLSAccess mplsAccess;

    // the collection of all label requests pending on the current LSR
    // from upstream LSRs and itself.
    typedef vector<fec_src_bind> FecSenderBindVector;
    FecSenderBindVector fecSenderBinds;

    // the collection of all HELLO adjacencies.
    typedef vector<peer_info> PeerVector;
    PeerVector myPeers;

    IPAddress local_addr;
    string id;
    double helloTimeout;  // FIXME obey
    bool isIR;
    bool isER;

    // holds TCP connections with peers
    TCPSocketMap socketMap;

    // hello timeout message
    cMessage *sendHelloMsg;

  private:
    /**
     * This method finds next peer in upstream direction
     */
    IPAddress locateNextHop(int fec);

    /**
     * This method maps the peerIP with the interface name in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    IPAddress findPeerAddrFromInterface(string interfaceName);

    //This method is the reserve of above method
    string findInterfaceFromPeerAddr(IPAddress peerIP);

  public:
    Module_Class_Members(NewLDP,cSimpleModule, 0);

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void broadcastHello();
    void openTCPConnectionToPeer(IPAddress addr);

    void processLDPHello(LDPHello *msg);
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


