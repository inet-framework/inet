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
#ifndef __LDPproc_H__
#define __LDPproc_H__



#include <string>
#include <omnetpp.h>
#include <iostream>
#include <vector>
#include "tcp.h"
#include "RoutingTable.h"
#include "LDPpacket.h"
#include "LDPInterface.h"
#include "LIBTableAccess.h"
#include "MPLSAccess.h"


//Parameter
//timeout, appl_timeout, peerNo, local_addr


//Make sure the following does not overlapped with TCP kinds
enum ldp_to_interface
{
    LDP_CLIENT_CREATE=20,
    LDP_FORWARD_REQUEST,
    LDP_RETURN_REPLY,
    LDP_BROADCAST_REQUEST

};

/**
 * LDP process.
 */
class LDPproc: public cSimpleModule
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
        int peerIP;
        string peerID;
        string role;
        string linkInterface;
    };

  private:
    RoutingTableAccess routingTableAccess;
    LIBTableAccess libTableAccess;
    MPLSAccess mplsAccess;

    //FecSenderBinds is the collection of all label requests pending on
    //the current LSR from upstream LSRs and itself.
    vector<fec_src_bind> FecSenderBinds;

    //myPeers is the collection of all HELLO adjacencies.
    vector<peer_info> myPeers;

    //myOwnFecRequest is the collection of all label requests pending on
    // the current LSR originated by itself
    //vector<fec_src_bind> myOwnFecRequest;

    int peerNo;
    int local_addr;
    string id;
    double helloTimeout;
    bool isIR;
    bool isER;

    /**
     * This method finds next peer in upstream direction
     */
    int locateNextHop(int fec);

    /**
     * This method maps the peerIP with the interface name in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    int findPeerAddrFromInterface(string interfaceName);

    //This method is the reserve of above method
    string findInterfaceFromPeerAddr(int peerIP);

  public:
    Module_Class_Members(LDPproc,cSimpleModule,16384);

    virtual void initialize();
    virtual void activity();

    void processRequestFromMPLSSwitch(cMessage *msg);
    void processLDPPacketFromTCP(LDPPacket *ldpPacket);

    void processLABEL_MAPPING(LDPLabelMapping *packet);
    void processLABEL_REQUEST(LDPLabelRequest *packet);
};

#endif


