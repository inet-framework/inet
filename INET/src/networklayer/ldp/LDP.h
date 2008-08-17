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

#ifndef __INET_LDP_H
#define __INET_LDP_H


#include <string>
#include <omnetpp.h>
#include <iostream>
#include <vector>
#include "INETDefs.h"
#include "LDPPacket_m.h"
#include "UDPSocket.h"
#include "TCPSocket.h"
#include "TCPSocketMap.h"
#include "IClassifier.h"
#include "NotificationBoard.h"

#define LDP_PORT  646

#define LDP_TRAFFIC         4       // session (TCP) traffic
#define LDP_HELLO_TRAFFIC   5       // discovery (UDP) traffic
#define LDP_USER_TRAFFIC    100     // label switched user traffic


class IInterfaceTable;
class IRoutingTable;
class LIBTable;
class TED;


/**
 * LDP (rfc 3036) protocol implementation.
 */
class INET_API LDP: public cSimpleModule, public TCPSocket::CallbackInterface, public IClassifier, public INotifiable
{
  public:

    struct fec_t
    {
        int fecid;

        // FEC value
        IPAddress addr;
        int length;

        // FEC's next hop address
        IPAddress nextHop;

        // possibly also: (speed up)
        // std::string nextHopInterface
    };
    typedef std::vector<fec_t> FecVector;


    struct fec_bind_t
    {
        int fecid;

        IPAddress peer;
        int label;
    };
    typedef std::vector<fec_bind_t> FecBindVector;


    struct pending_req_t
    {
        int fecid;
        IPAddress peer;
    };
    typedef std::vector<pending_req_t> PendingVector;

    struct peer_info
    {
        IPAddress peerIP;   // IP address of LDP peer
        bool activeRole;    // we're in active or passive role in this session
        TCPSocket *socket;  // TCP socket
        std::string linkInterface;
        cMessage *timeout;
    };
    typedef std::vector<peer_info> PeerVector;

  protected:
    // configuration
    simtime_t holdTime;
    simtime_t helloInterval;

    // currently recognized FECs
    FecVector fecList;
    // bindings advertised upstream
    FecBindVector fecUp;
    // mappings learnt from downstream
    FecBindVector fecDown;
    // currently requested and yet unserviced mappings
    PendingVector pending;

    // the collection of all HELLO adjacencies.
    PeerVector myPeers;

    //
    // other variables:
    //
    IInterfaceTable *ift;
    IRoutingTable *rt;
    LIBTable *lt;
    TED *tedmod;
    NotificationBoard *nb;

    UDPSocket udpSocket;     // for sending/receiving Hello
    TCPSocket serverSocket;  // for listening on LDP_PORT
    TCPSocketMap socketMap;  // holds TCP connections with peers

    // hello timeout message
    cMessage *sendHelloMsg;

    int maxFecid;

  protected:
    /**
     * This method finds next peer in upstream direction
     */
    virtual IPAddress locateNextHop(IPAddress dest);

    /**
     * This method maps the peerIP with the interface name in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    virtual IPAddress findPeerAddrFromInterface(std::string interfaceName);

    //This method is the reserve of above method
    std::string findInterfaceFromPeerAddr(IPAddress peerIP);

    /** Utility: return peer's index in myPeers table, or -1 if not found */
    virtual int findPeer(IPAddress peerAddr);

    /** Utility: return socket for given peer. Throws error if there's no TCP connection */
    virtual TCPSocket *getPeerSocket(IPAddress peerAddr);

    /** Utility: return socket for given peer, and NULL if session doesn't exist */
    virtual TCPSocket *findPeerSocket(IPAddress peerAddr);

    virtual void sendToPeer(IPAddress dest, cMessage *msg);


    //bool matches(const FEC_TLV& a, const FEC_TLV& b);

    FecVector::iterator findFecEntry(FecVector& fecs, IPAddress addr, int length);
    FecBindVector::iterator findFecEntry(FecBindVector& fecs, int fecid, IPAddress peer);

    virtual void sendMappingRequest(IPAddress dest, IPAddress addr, int length);
    virtual void sendMapping(int type, IPAddress dest, int label, IPAddress addr, int length);
    virtual void sendNotify(int status, IPAddress dest, IPAddress addr, int length);

    virtual void rebuildFecList();
    virtual void updateFecList(IPAddress nextHop);
    virtual void updateFecListEntry(fec_t oldItem);

    virtual void announceLinkChange(int tedlinkindex);

  public:
    LDP();
    virtual ~LDP();

  protected:
    virtual int numInitStages() const  {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void sendHelloTo(IPAddress dest);
    virtual void openTCPConnectionToPeer(int peerIndex);

    virtual void processLDPHello(LDPHello *msg);
    virtual void processHelloTimeout(cMessage *msg);
    virtual void processMessageFromTCP(cMessage *msg);
    virtual void processLDPPacketFromTCP(LDPPacket *ldpPacket);

    virtual void processLABEL_MAPPING(LDPLabelMapping *packet);
    virtual void processLABEL_REQUEST(LDPLabelRequest *packet);
    virtual void processLABEL_RELEASE(LDPLabelMapping *packet);
    virtual void processLABEL_WITHDRAW(LDPLabelMapping *packet);
    virtual void processNOTIFICATION(LDPNotify* packet);

    /** @name TCPSocket::CallbackInterface callback methods */
    //@{
    virtual void socketEstablished(int connId, void *yourPtr);
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent);
    virtual void socketPeerClosed(int connId, void *yourPtr);
    virtual void socketClosed(int connId, void *yourPtr);
    virtual void socketFailure(int connId, void *yourPtr, int code);
    virtual void socketStatusArrived(int connId, void *yourPtr, TCPStatusInfo *status) {delete status;}
    //@}

    // IClassifier
    virtual bool lookupLabel(IPDatagram *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color);

    // INotifiable
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
};

#endif


