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
#include "inet/networklayer/mpls/LibTable.h"
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

// RFC 5036 Section 2.8/3.4.4-3.4.5: loop detection TLVs, optionally carried by Label
// Request and Label Mapping messages when Ldp.loopDetection is enabled (see
// Ldp::sendMappingRequest/sendMapping and LdpPacketSerializer).
const B LDP_HOP_COUNT_TLV_BYTES = B(4 + 1); // Hop Count TLV: 4-byte TLV header + 1-byte HC Value
const B LDP_PATH_VECTOR_TLV_HEADER_BYTES = B(4); // Path Vector TLV header; the variable part (4 bytes per LSR-ID) is added separately

// RFC 5036 Section 3.5.3: Common Session Parameters TLV (used by the Initialization message)
const B LDP_COMMON_SESSION_PARAMETERS_TLV_BYTES = B(20);

// RFC 5036 Section 3.4.2: Address List TLV header (4-byte TLV header + 2-byte
// address family); the variable part (4 bytes per address) is added separately
// since the Address/Address Withdraw message length depends on the number of
// addresses advertised (see Ldp::sendAddress).
const B LDP_ADDRESS_LIST_TLV_HEADER_BYTES = B(4 + 2);

// Model simplification (see Ldp.ned): exactly one message per PDU, so each
// packet's total length is the PDU header plus one message (message header + TLVs).
const B LDP_HELLO_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_COMMON_HELLO_PARAMETERS_TLV_BYTES; // 26 B
const B LDP_LABEL_REQUEST_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_FEC_TLV_BYTES; // 30 B
// Label Mapping, and also Label Withdraw/Release (sendMapping() reuses the same wire fields -- FEC + label -- for all three message types)
const B LDP_LABEL_MAPPING_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_FEC_TLV_BYTES + LDP_GENERIC_LABEL_TLV_BYTES; // 38 B
const B LDP_NOTIFICATION_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_STATUS_TLV_BYTES + LDP_FEC_TLV_BYTES; // 44 B; sendNotify() always includes a FEC TLV
const B LDP_INITIALIZATION_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_COMMON_SESSION_PARAMETERS_TLV_BYTES; // 38 B
const B LDP_KEEPALIVE_BYTES = LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES; // 18 B; KeepAlive carries no message parameters (RFC 5036 Section 3.5.4)

// Address/Address Withdraw message length depends on the number of addresses
// carried (see Ldp::sendAddress); also used by LdpPacketSerializer indirectly
// (the wire length is derived from chunk length, not recomputed there).
inline B ldpAddressMessageBytes(size_t numAddresses)
{
    return LDP_PDU_HEADER_BYTES + LDP_MESSAGE_HEADER_BYTES + LDP_ADDRESS_LIST_TLV_HEADER_BYTES + B(4 * numAddresses);
}

// Extra bytes contributed by the loop detection Hop Count + Path Vector TLVs
// (RFC 5036 Section 2.8), added on top of LDP_LABEL_REQUEST_BYTES/LDP_LABEL_MAPPING_BYTES
// when Ldp.loopDetection is enabled and the message actually carries them (see
// Ldp::sendMappingRequest/sendMapping); with loopDetection disabled (the default) these
// TLVs are never populated and the base message length is unchanged.
inline B ldpLoopDetectionTlvBytes(size_t pathVectorLength)
{
    return LDP_HOP_COUNT_TLV_BYTES + LDP_PATH_VECTOR_TLV_HEADER_BYTES + B(4 * pathVectorLength);
}

class IInterfaceTable;
class IIpv4RoutingTable;
class Ted;

/**
 * LDP (rfc 3036) protocol implementation.
 */
class INET_API Ldp : public RoutingProtocolBase, public TcpSocket::BufferingCallback, public UdpSocket::ICallback, public cListener
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

        // Only meaningful for fecUp entries under DU/independent control: false
        // while 'label' has been advertised upstream but is only a reservation
        // (LibTable::allocateLabel()), with no LIB entry yet -- the downstream
        // mapping needed to complete the swap (LibTable::installReservedLabel())
        // hasn't arrived (see Ldp::duAdvertiseToPeer/processLABEL_MAPPING).
        // Always true for fecDown entries and for the DoD path, which never
        // defers installation.
        bool installed = true;

        // RFC 5036 Section 2.8 loop detection (only meaningful when Ldp.loopDetection
        // is enabled): for a fecDown entry, the Hop Count/Path Vector carried by the
        // Label Mapping that established this binding -- re-advertising it to another
        // peer (see Ldp::duAdvertiseToPeer/processLABEL_MAPPING) forwards this same
        // vector, incremented and extended with our own LSR-ID, via Ldp::sendMapping.
        // Left at its default (0/empty) for fecUp entries, which are never themselves
        // re-forwarded further.
        uint8_t hopCount = 0;
        std::vector<Ipv4Address> pathVector;
    };
    typedef std::vector<FecBinding> FecBindVector;

    struct PendingRequest {
        int fecid;
        Ipv4Address peer;
    };
    typedef std::vector<PendingRequest> PendingVector;

    struct peer_info {
        // LDP session FSM (RFC 5036 Section 2.5.3-2.5.5): NONEXISTENT until the TCP
        // connection is established; OPENSENT/OPENREC while Initialization/KeepAlive
        // are being exchanged; OPERATIONAL once the 4-way handshake completes. Label
        // and address messages are only valid once OPERATIONAL (see
        // Ldp::processLdpPacketFromTcp).
        enum SessionState { NONEXISTENT, INITIALIZED, OPENSENT, OPENREC, OPERATIONAL };

        Ipv4Address peerIP; // Ipv4 address of LDP peer
        bool activeRole; // we're in active or passive role in this session
        TcpSocket *socket; // TCP socket
        std::string linkInterface; // name of the local interface the Hello adjacency was formed on; also the authoritative answer to "which local interface reaches this peer" (see Ldp::findInterfaceFromPeerAddr)
        cMessage *timeout;

        // RFC 5036 Section 3.5.5/2.7: interface addresses this peer advertised via
        // Address messages (Address Withdraw removes entries); populated once the
        // session is OPERATIONAL (see Ldp::processADDRESS/processADDRESS_WITHDRAW).
        // Together with peerIP (the peer's LSR-ID) this is the authoritative
        // "which address belongs to this peer session" map -- see
        // Ldp::findInterfaceFromPeerAddr, which consults it instead of guessing
        // through a generic routing-table lookup.
        std::vector<Ipv4Address> peerAddresses;

        SessionState state = NONEXISTENT;
        simtime_t negotiatedKeepaliveTime = 0; // min(ours, peer's KeepAlive Time) once negotiated; 0 until then
        // KeepAlive-based session liveness (RFC 5036 Section 2.5.6), scheduled only
        // once the session is OPERATIONAL: keepAliveSendTimer fires every
        // negotiatedKeepaliveTime/3 without an LDP message having been SENT to this
        // peer (reset in Ldp::sendToPeer on every send) and triggers an explicit
        // KeepAlive; sessionHoldTimer fires negotiatedKeepaliveTime after the last
        // LDP message RECEIVED from this peer (reset in
        // Ldp::processLdpPacketFromTcp on every receive) and tears the session down
        // (Ldp::processSessionHoldTimeout). This is deliberately a narrower, separate
        // failure mode from the Hello hold-timer's adjacency-level death detection
        // in Ldp::processHelloTimeout: that one detects "the peer/link is gone" via
        // UDP Hello; this one detects "the TCP control channel died while the Hello
        // adjacency is still alive" (a stale CONNECTED TcpSocket the transport layer
        // hasn't itself noticed yet) -- the two timeouts are not merged. Cleaned up
        // by every teardown path (see Ldp::removePeerBindings).
        cMessage *keepAliveSendTimer = nullptr;
        cMessage *sessionHoldTimer = nullptr;
    };
    typedef std::vector<peer_info> PeerVector;

  protected:
    // configuration
    simtime_t holdTime;
    simtime_t helloInterval;
    simtime_t keepaliveTime; // Session KeepAlive Time we propose in Initialization messages (RFC 5036 Section 3.5.3)
    bool advertiseImplicitNull = true;

    // RFC 5036 Section 2.6 label distribution mode parameters -- see Ldp.ned for
    // the full semantics; distributionMode also drives the Initialization
    // message's A-bit (see sendInit/processINITIALIZATION).
    std::string distributionMode; // "du" (default) or "dod"
    std::string controlMode; // "independent" (default) or "ordered"; only consulted when distributionMode=="du"
    std::string retentionMode; // "liberal" (default) or "conservative"

    // RFC 5036 Section 2.8 loop detection; see Ldp.ned. Disabled (loopDetection==false)
    // is the default and leaves Label Request/Mapping processing completely unchanged.
    bool loopDetection = false;
    int pathVectorLimit = 32;

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
    // Maps a peer-side address (a peer's LSR-ID, or one of its interface
    // addresses learned via an Address message -- see peer_info::peerAddresses)
    // to the local interface used to reach it. Consults the peer table (built
    // from real Hello adjacencies and Address messages) first, since that is
    // the authoritative source of "which peer session owns this address";
    // falls back to a routing-table lookup for addresses that are not (yet)
    // known to belong to any OPERATIONAL peer.
    int findInterfaceFromPeerAddr(Ipv4Address peerIP);

    /** Utility: return peer's index in myPeers table, or -1 if not found */
    virtual int findPeer(Ipv4Address peerAddr);

    /** Utility: return socket for given peer. Throws error if there's no TCP connection */
    virtual TcpSocket *getPeerSocket(Ipv4Address peerAddr);

    /** Utility: return socket for given peer, and nullptr if session doesn't exist */
    virtual TcpSocket *findPeerSocket(Ipv4Address peerAddr);

    virtual void sendToPeer(Ipv4Address dest, Packet *msg);

    /** Utility: true if we hold an OPERATIONAL LDP session with this peer (session FSM gate) */
    virtual bool isPeerOperational(Ipv4Address peerAddr);

    FecVector::iterator findFecEntry(FecVector& fecs, Ipv4Address addr, int length);
    FecBindVector::iterator findFecEntry(FecBindVector& fecs, int fecid, Ipv4Address peer);

    // label/binding-plane sends: RFC 5036 Section 3.5.3 -- only valid once the
    // session with 'dest' is OPERATIONAL; a no-op (EV_WARN) otherwise
    //
    // 'hopCount'/'pathVector' are only ever consulted when loopDetection is enabled
    // (see Ldp.ned): an empty 'pathVector' (the default -- used by every call site that
    // originates a request/mapping, as opposed to propagating one received from a peer)
    // means "we are the origin", so the message is sent with hopCount=1 and
    // pathVector=[our own LSR-ID]; a non-empty 'pathVector' (passed by call sites that
    // are propagating a request/mapping received from another peer -- see
    // processLABEL_REQUEST/processLABEL_MAPPING) means the message is sent with
    // hopCount+1 and 'pathVector' plus our own LSR-ID appended (RFC 5036 Section 2.8).
    virtual void sendMappingRequest(Ipv4Address dest, Ipv4Address addr, int length, uint8_t hopCount = 0, const std::vector<Ipv4Address>& pathVector = {});
    virtual void sendMapping(int type, Ipv4Address dest, int label, Ipv4Address addr, int length, uint8_t hopCount = 0, const std::vector<Ipv4Address>& pathVector = {});
    virtual void sendNotify(int status, Ipv4Address dest, Ipv4Address addr, int length);

    // RFC 5036 Section 2.8: true if 'pathVector' already contains our own LSR-ID, or
    // 'hopCount' has gone past pathVectorLimit -- either condition means a propagated
    // Label Request/Mapping has gone all the way around a forwarding loop. Only ever
    // called when loopDetection is enabled and the received message actually carries
    // loop-detection state.
    virtual bool isLoopDetected(uint8_t hopCount, const std::vector<Ipv4Address>& pathVector);

    // session establishment (RFC 5036 Section 2.5.3): not gated on OPERATIONAL --
    // these ARE the messages that bring the session to OPERATIONAL
    virtual void sendInit(Ipv4Address dest);
    virtual void sendKeepAlive(Ipv4Address dest);

    // RFC 5036 Section 3.5.5/2.7: advertise all of this router's LDP-capable
    // interface addresses to a newly-OPERATIONAL peer (see processKEEPALIVE)
    virtual void sendAddress(Ipv4Address dest);

    virtual void rebuildFecList();
    virtual void updateFecList(Ipv4Address nextHop);
    virtual void updateFecListEntry(Fec oldItem);

    // RFC 5036 Section 2.6: Downstream Unsolicited -- (re)advertise our label
    // mapping for 'fec' to a single OPERATIONAL 'peer', deciding egress/
    // independent-control/ordered-control/liberal-retention-switchover behavior
    // from the current state (see Ldp.cc for the full decision tree). Called (a)
    // once per FEC when a session first reaches OPERATIONAL (processKEEPALIVE) and
    // (b) once per OPERATIONAL peer whenever updateFecListEntry (called only on
    // FEC creation or a next-hop change, never for an unchanged FEC) detects
    // distributionMode=="du".
    virtual void duAdvertiseToPeer(const Fec& fec, Ipv4Address peer);

    // emits the current total binding count (fecUp.size() + fecDown.size()) on the fecBindingCount signal
    virtual void emitFecBindingCount();

  public:
    Ldp();
    virtual ~Ldp();

    // lifecycle
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    /**
     * Ingress classification decision for an incoming Ipv4 datagram: longest-prefix
     * match over fecList, followed by a fecDown lookup for the matched FEC, plus the
     * LDP/OSPF port skip-rules. Called by the sibling ~LdpClassifier module, which
     * implements IIngressClassifier and delegates to this method rather than
     * maintaining its own bind-time FEC/label table -- see LdpClassifier.h for why.
     */
    virtual bool classifyPacket(Packet *ipdatagram, LabelOpVector& outLabel, int& outInterfaceId);

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
    // KeepAlive-based session liveness (RFC 5036 Section 2.5.6); see peer_info's
    // keepAliveSendTimer/sessionHoldTimer fields for the division of labor
    virtual void processKeepAliveSendTimeout(cMessage *msg);
    virtual void processSessionHoldTimeout(cMessage *msg);
    virtual void processLdpPacketFromTcp(Ptr<const LdpPacket>& ldpPacket);

    virtual void processINITIALIZATION(Ptr<const LdpPacket>& ldpPacket);
    virtual void processKEEPALIVE(Ptr<const LdpPacket>& ldpPacket);
    virtual void processADDRESS(Ptr<const LdpPacket>& ldpPacket);
    virtual void processADDRESS_WITHDRAW(Ptr<const LdpPacket>& ldpPacket);
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

    // cListener
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

