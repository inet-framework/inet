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
        IPv4Address addr;
        int length;

        // FEC's next hop address
        IPv4Address nextHop;

        // possibly also: (speed up)
        // std::string nextHopInterface
    };
    typedef std::vector<fec_t> FecVector;


    struct fec_bind_t
    {
        int fecid;

        IPv4Address peer;
        int label;
    };
    typedef std::vector<fec_bind_t> FecBindVector;


    struct pending_req_t
    {
        int fecid;
        IPv4Address peer;
    };
    typedef std::vector<pending_req_t> PendingVector;

    struct peer_info
    {
        IPv4Address peerIP;   // IPv4 address of LDP peer
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

    UDPSocket udpSocket; // for receiving Hello
    std::vector<UDPSocket> udpSockets;  // for sending Hello, one socket for each multicast interface
    TCPSocket serverSocket;  // for listening on LDP_PORT
    TCPSocketMap socketMap;  // holds TCP connections with peers

    // hello timeout message
    cMessage *sendHelloMsg;

    int maxFecid;

  protected:
    /**
     * This method finds next peer in upstream direction
     */
    virtual IPv4Address locateNextHop(IPv4Address dest);

    /**
     * This method maps the peerIP with the interface name in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    virtual IPv4Address findPeerAddrFromInterface(std::string interfaceName);

    //This method is the reserve of above method
    std::string findInterfaceFromPeerAddr(IPv4Address peerIP);

    /** Utility: return peer's index in myPeers table, or -1 if not found */
    virtual int findPeer(IPv4Address peerAddr);

    /** Utility: return socket for given peer. Throws error if there's no TCP connection */
    virtual TCPSocket *getPeerSocket(IPv4Address peerAddr);

    /** Utility: return socket for given peer, and NULL if session doesn't exist */
    virtual TCPSocket *findPeerSocket(IPv4Address peerAddr);

    virtual void sendToPeer(IPv4Address dest, cMessage *msg);


    //bool matches(const FEC_TLV& a, const FEC_TLV& b);

    FecVector::iterator findFecEntry(FecVector& fecs, IPv4Address addr, int length);
    FecBindVector::iterator findFecEntry(FecBindVector& fecs, int fecid, IPv4Address peer);

    virtual void sendMappingRequest(IPv4Address dest, IPv4Address addr, int length);
    virtual void sendMapping(int type, IPv4Address dest, int label, IPv4Address addr, int length);
    virtual void sendNotify(int status, IPv4Address dest, IPv4Address addr, int length);

    virtual void rebuildFecList();
    virtual void updateFecList(IPv4Address nextHop);
    virtual void updateFecListEntry(fec_t oldItem);

    virtual void announceLinkChange(int tedlinkindex);

  public:
    LDP();
    virtual ~LDP();

  protected:
    virtual int numInitStages() const  {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void sendHelloTo(IPv4Address dest);
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
    virtual bool lookupLabel(IPv4Datagram *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color);

    // INotifiable
    virtual void receiveChangeNotification(int category, const cObject *details);
};

#endif


