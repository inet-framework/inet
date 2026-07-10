//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2004 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_LDP_H
#define __INET_LDP_H

#include <iostream>
#include <string>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/socket/SocketMap.h"
#include "inet/networklayer/ldp/LdpPacket_m.h"
#include "inet/networklayer/mpls/IIngressClassifier.h"
#include "inet/routing/base/RoutingProtocolBase.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

#define LDP_PORT             646

// RFC 5036 Section 3.5.3: PDU common header (version 2 + PDU length 2 + LSR-ID 4 + label space 2)
const B LDP_PDU_HEADER_BYTES = B(10);
// RFC 5036 Section 3.6: message header (U-bit + message type 2 + message length 2 + message id 4)
const B LDP_MESSAGE_HEADER_BYTES = B(8);

// TLV sizes (4-byte TLV header + value), RFC 5036 Section 3.4
const B LDP_FEC_TLV_BYTES = B(4 + 8); // one IPv4-prefix FEC element (element type 1 + address family 2 + prefix length 1 + 4 prefix bytes); this model's FecTlv always carries exactly one element
const B LDP_GENERIC_LABEL_TLV_BYTES = B(8);
const B LDP_COMMON_HELLO_PARAMETERS_TLV_BYTES = B(8);
const B LDP_STATUS_TLV_BYTES = B(14);

// Model simplification (see Ldp.ned): exactly one message per PDU, so each
// packet's total length is the PDU header plus one message (message header + TLVs).
const B LDP_HELLO_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_COMMON_HELLO_PARAMETERS_TLV_BYTES; // 26 B
const B LDP_LABEL_REQUEST_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_FEC_TLV_BYTES; // 30 B
// Label Mapping, and also Label Withdraw/Release (sendMapping() reuses the same wire fields -- FEC + label -- for all three message types)
const B LDP_LABEL_MAPPING_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_FEC_TLV_BYTES + LDP_GENERIC_LABEL_TLV_BYTES; // 38 B
const B LDP_NOTIFICATION_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_STATUS_TLV_BYTES + LDP_FEC_TLV_BYTES; // 44 B; sendNotify() always includes a FEC TLV

class IInterfaceTable;
class IIpv4RoutingTable;
class LibTable;
class Ted;

/**
 * LDP (rfc 3036) protocol implementation.
 */
class INET_API Ldp : public RoutingProtocolBase, public TcpSocket::BufferingCallback, public UdpSocket::ICallback, public IIngressClassifier, public cListener
{
  public:

    struct Fec {
        int fecid;

        // FEC value
        Ipv4Address addr;
        int length;

        // FEC's next hop address
        Ipv4Address nextHop;

        // possibly also: (speed up)
//        std::string nextHopInterface
    };
    typedef std::vector<Fec> FecVector;

    struct FecBinding {
        int fecid;

        Ipv4Address peer;
        int label;
    };
    typedef std::vector<FecBinding> FecBindVector;

    struct PendingRequest {
        int fecid;
        Ipv4Address peer;
    };
    typedef std::vector<PendingRequest> PendingVector;

    struct peer_info {
        Ipv4Address peerIP; // Ipv4 address of LDP peer
        bool activeRole; // we're in active or passive role in this session
        TcpSocket *socket; // TCP socket
        std::string linkInterface;
        cMessage *timeout;
    };
    typedef std::vector<peer_info> PeerVector;

  protected:
    // configuration
    simtime_t holdTime;
    simtime_t helloInterval;
    bool advertiseImplicitNull = true;

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
    ModuleRefByPar<IInterfaceTable> ift;
    ModuleRefByPar<IIpv4RoutingTable> rt;
    ModuleRefByPar<LibTable> lt;
    ModuleRefByPar<Ted> tedmod;

    UdpSocket udpSocket; // for receiving Hello
    std::vector<UdpSocket> udpSockets; // for sending Hello, one socket for each multicast interface
    TcpSocket serverSocket; // for listening on LDP_PORT
    SocketMap socketMap; // holds TCP connections with peers
    std::vector<TcpSocket *> deadSockets; // sockets torn down in a callback, deleted later

    // hello timeout message
    cMessage *sendHelloMsg = nullptr;
    std::vector<cMessage *> retryMsgs; // scheduled NOTIFICATION-retry self-messages

    int maxFecid = 0;
    long numSent = 0;
    long numReceived = 0;

    static simsignal_t sessionUpSignal;
    static simsignal_t sessionDownSignal;
    static simsignal_t fecBindingCountSignal;

  protected:
    /**
     * This method finds next peer in upstream direction
     */
    virtual Ipv4Address locateNextHop(Ipv4Address dest);

    /**
     * This method maps the peerIP with the interface id in routing table.
     * It is expected that for MPLS host, entries linked to MPLS peers are available.
     * In case no corresponding peerIP found, a peerIP (not deterministic)
     * will be returned.
     */
    virtual Ipv4Address findPeerAddrFromInterface(int interfaceId);

    // This method is the reserve of above method
    int findInterfaceFromPeerAddr(Ipv4Address peerIP);

    /** Utility: return peer's index in myPeers table, or -1 if not found */
    virtual int findPeer(Ipv4Address peerAddr);

    /** Utility: return socket for given peer. Throws error if there's no TCP connection */
    virtual TcpSocket *getPeerSocket(Ipv4Address peerAddr);

    /** Utility: return socket for given peer, and nullptr if session doesn't exist */
    virtual TcpSocket *findPeerSocket(Ipv4Address peerAddr);

    virtual void sendToPeer(Ipv4Address dest, Packet *msg);

    FecVector::iterator findFecEntry(FecVector& fecs, Ipv4Address addr, int length);
    FecBindVector::iterator findFecEntry(FecBindVector& fecs, int fecid, Ipv4Address peer);

    virtual void sendMappingRequest(Ipv4Address dest, Ipv4Address addr, int length);
    virtual void sendMapping(int type, Ipv4Address dest, int label, Ipv4Address addr, int length);
    virtual void sendNotify(int status, Ipv4Address dest, Ipv4Address addr, int length);

    virtual void rebuildFecList();
    virtual void updateFecList(Ipv4Address nextHop);
    virtual void updateFecListEntry(Fec oldItem);

    // emits the current total binding count (fecUp.size() + fecDown.size()) on the fecBindingCount signal
    virtual void emitFecBindingCount();

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

    virtual void setupSockets();
    virtual void clearState();
    virtual void sendHelloTo(Ipv4Address dest);
    virtual void openTCPConnectionToPeer(int peerIndex);
    virtual void removePeerBindings(Ipv4Address peerIP);
    virtual void handleTcpConnectionDown(TcpSocket *socket);

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
    virtual void socketStatusArrived(TcpSocket *socket, TcpStatusInfo *status) override {}
    virtual void socketDeleted(TcpSocket *socket) override {} // TODO
    //@}

    /** @name UdpSocket::ICallback methods */
    //@{
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;
    //@}

    // IIngressClassifier
    virtual bool lookupLabel(Packet *ipdatagram, LabelOpVector& outLabel, int& outInterfaceId) override;

    // cListener
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

