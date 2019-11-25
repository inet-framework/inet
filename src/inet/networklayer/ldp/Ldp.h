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

#include <iostream>
#include <string>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/networklayer/ldp/LdpPacket_m.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

#define LDP_PORT             646

#define LDP_TRAFFIC          4       // session (TCP) traffic
#define LDP_HELLO_TRAFFIC    5       // discovery (UDP) traffic
#define LDP_USER_TRAFFIC     100     // label switched user traffic

// base header: version, length, LSR ID, Label space
const B LDP_BASEHEADER_BYTES = B(10);

// FIXME: the length below is just a guess. TBD find lengths for individual TLVs
// making up different LDP packet types, and determine length for each packet type
const B LDP_HEADER_BYTES = LDP_BASEHEADER_BYTES + B(20);

class IInterfaceTable;
class IIpv4RoutingTable;
class LibTable;
class Ted;

/**
 * LDP (rfc 3036) protocol implementation.
 */
class INET_API Ldp : public RoutingProtocolBase, public TcpSocket::ReceiveQueueBasedCallback, public UdpSocket::ICallback, public IIngressClassifier, public cListener
{
  public:

    struct fec_t
    {
        int fecid;

        // FEC value
        Ipv4Address addr;
        int length;

        // FEC's next hop address
        Ipv4Address nextHop;

        // possibly also: (speed up)
        // std::string nextHopInterface
    };
    typedef std::vector<fec_t> FecVector;

    struct fec_bind_t
    {
        int fecid;

        Ipv4Address peer;
        int label;
    };
    typedef std::vector<fec_bind_t> FecBindVector;

    struct pending_req_t
    {
        int fecid;
        Ipv4Address peer;
    };
    typedef std::vector<pending_req_t> PendingVector;

    struct peer_info
    {
        Ipv4Address peerIP;    // Ipv4 address of LDP peer
        bool activeRole;    // we're in active or passive role in this session
        TcpSocket *socket;    // TCP socket
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
    IInterfaceTable *ift = nullptr;
    IIpv4RoutingTable *rt = nullptr;
    LibTable *lt = nullptr;
    Ted *tedmod = nullptr;

    UdpSocket udpSocket;    // for receiving Hello
    std::vector<UdpSocket> udpSockets;    // for sending Hello, one socket for each multicast interface
    TcpSocket serverSocket;    // for listening on LDP_PORT
    SocketMap socketMap;    // holds TCP connections with peers

    // hello timeout message
    cMessage *sendHelloMsg = nullptr;

    int maxFecid = 0;

  protected:
    /**
     * This method finds next peer in upstream direction
     */
    virtual Ipv4Address locateNextHop(Ipv4Address dest);

    /**
     * This method maps the peerIP with the interface name in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    virtual Ipv4Address findPeerAddrFromInterface(std::string interfaceName);

    //This method is the reserve of above method
    std::string findInterfaceFromPeerAddr(Ipv4Address peerIP);

    /** Utility: return peer's index in myPeers table, or -1 if not found */
    virtual int findPeer(Ipv4Address peerAddr);

    /** Utility: return socket for given peer. Throws error if there's no TCP connection */
    virtual TcpSocket *getPeerSocket(Ipv4Address peerAddr);

    /** Utility: return socket for given peer, and nullptr if session doesn't exist */
    virtual TcpSocket *findPeerSocket(Ipv4Address peerAddr);

    virtual void sendToPeer(Ipv4Address dest, Packet *msg);

    //bool matches(const FecTlv& a, const FecTlv& b);

    FecVector::iterator findFecEntry(FecVector& fecs, Ipv4Address addr, int length);
    FecBindVector::iterator findFecEntry(FecBindVector& fecs, int fecid, Ipv4Address peer);

    virtual void sendMappingRequest(Ipv4Address dest, Ipv4Address addr, int length);
    virtual void sendMapping(int type, Ipv4Address dest, int label, Ipv4Address addr, int length);
    virtual void sendNotify(int status, Ipv4Address dest, Ipv4Address addr, int length);

    virtual void rebuildFecList();
    virtual void updateFecList(Ipv4Address nextHop);
    virtual void updateFecListEntry(fec_t oldItem);

    virtual void announceLinkChange(int tedlinkindex);

  public:
    Ldp();
    virtual ~Ldp();

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;

    virtual void sendHelloTo(Ipv4Address dest);
    virtual void openTCPConnectionToPeer(int peerIndex);

    virtual void processLDPHello(Packet *msg);
    virtual void processHelloTimeout(cMessage *msg);
    virtual void processLdpPacketFromTcp(Ptr<const LdpPacket>& ldpPacket);

    virtual void processLABEL_MAPPING(Ptr<const LdpPacket>& ldpPacket);
    virtual void processLABEL_REQUEST(Ptr<const LdpPacket>& ldpPacket);
    virtual void processLABEL_RELEASE(Ptr<const LdpPacket>& ldpPacket);
    virtual void processLABEL_WITHDRAW(Ptr<const LdpPacket>& ldpPacket);
    virtual void processNOTIFICATION(Ptr<const LdpPacket>& ldpPacket, bool rescheduled);

    /** @name TcpSocket::ICallback callback methods */
    //@{
    virtual void socketDataArrived(TcpSocket *socket) override;
    virtual void socketAvailable(TcpSocket *socket, TcpAvailableInfo *availableInfo) override;
    virtual void socketEstablished(TcpSocket *socket) override;
    virtual void socketPeerClosed(TcpSocket *socket) override;
    virtual void socketClosed(TcpSocket *socket) override;
    virtual void socketFailure(TcpSocket *socket, int code) override;
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override { }
    virtual void socketDeleted(TcpSocket *socket) override {}   //TODO
    //@}

    /** @name UdpSocket::ICallback methods */
    //@{
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
    //@}

    // IIngressClassifier
    virtual bool lookupLabel(Packet *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color) override;

    // cListener
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif // ifndef __INET_LDP_H

