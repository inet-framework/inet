//
// Copyright (C) 2005 Vojta Janota
// Copyright (C) 2004 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.ldp;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// Handles and processes LDP messages.
//
// The LDP protocol is used by one LSR (Label Switched Router) to inform
// another LSR of the label bindings it has made. The LSR uses this protocol
// to establish label switched paths through a network by mapping network layer
// routing information directly to data-link layer switched paths.
//
// Operations
//
// All LDP message types are subclassed from ~LdpPacket, and include
// ~LdpHello, ~LdpNotify, ~LdpLabelRequest, ~LdpIni, ~LdpKeepAlive and ~LdpAddress.
//
// Message processing in the ~Ldp module is explained in
// <a href="ldp-processing.html">LDP Message Processing</a>
//
// Model simplification: RFC 5036 allows a PDU to carry multiple LDP messages;
// this model always sends exactly one message per PDU, so the chunk length
// computed for each outgoing packet is simply the 10-byte PDU header plus the
// length of that single message (its 8-byte message header plus its TLVs).
//
// Session establishment (RFC 5036 Section 2.5.2-2.5.5): once the TCP connection
// is established, the active/passive LSRs exchange Initialization and KeepAlive
// messages (~LdpIni, ~LdpKeepAlive) to negotiate the session KeepAlive Time
// (min of both proposals); the session is usable for label/address messages
// only once this handshake completes (state OPERATIONAL). Once OPERATIONAL,
// KeepAlive-based session liveness (RFC 5036 Section 2.5.6) keeps the session
// alive: a KeepAlive is sent whenever a full negotiatedKeepaliveTime/3 elapses
// without any LDP message having been sent to the peer, and the session is
// torn down (Notification + transport connection abort + discarded bindings)
// if a full negotiatedKeepaliveTime elapses without any LDP message being
// received from the peer. This is deliberately independent from the Hello
// hold-timer's adjacency-level death detection (~holdTime): that one detects
// "the peer/link is gone" via UDP Hello, this one detects "the TCP control
// channel died" while the Hello adjacency may still be alive -- when only the
// latter happens, the surviving Hello adjacency re-establishes the session on
// the next Hello (see Ldp::processLDPHello).
//
// Address / Address Withdraw messages (RFC 5036 Section 3.5.5/2.7): once a
// session reaches OPERATIONAL, each side advertises all of its LDP-capable
// interface addresses to the other (~LdpAddress); the receiver stores them
// per-peer and uses that table -- together with the peer's LSR-ID -- as the
// authoritative "which peer session owns this address" map (see
// Ldp::findInterfaceFromPeerAddr), instead of guessing through a routing-table
// lookup. Explicitly NOT modeled: interface addresses added/removed while a
// session is up are never re-advertised (no Address Withdraw is ever sent by
// this module, only received/processed) -- the address table is only ever
// (re)built at session establishment time; a config change requiring a
// mid-session Address Withdraw is out of scope.
//
// Label distribution modes (RFC 5036 Section 2.6): ~distributionMode,
// ~controlMode and ~retentionMode select, independently, the advertisement
// discipline, the control mode, and the retention mode. The DEFAULT
// combination -- Downstream Unsolicited, independent control, liberal
// retention -- is the real-world/RFC-conventional deployment default (a
// deliberate compliance choice, not an oversight): every OPERATIONAL peer is
// proactively sent Label Mappings for every FEC this router can serve,
// without waiting to be asked, and mappings received from a peer that is not
// (or not yet) a FEC's next hop are kept rather than discarded, enabling
// instant failover on a next-hop change (see Ldp::duAdvertiseToPeer). The
// original Downstream-on-Demand / ordered-control / (implicit) conservative-
// retention behavior remains fully selectable via the same three parameters
// and is otherwise unchanged.
//
// Loop detection (RFC 5036 Section 2.8): ~loopDetection, disabled by default. When
// enabled, propagated Label Request messages (downstream) and Label Mapping messages
// (upstream) carry a Hop Count TLV and a Path Vector TLV (the list of LSR-IDs the
// message has traversed); a receiving LSR that finds its own LSR-ID already in the
// path vector, or a hop count beyond ~pathVectorLimit, has detected a forwarding
// loop -- it sends a Notification (Loop Detected status) back to the sender instead
// of propagating or accepting the message.
//
// Explicit non-features (RFC 5036 mechanisms not modeled by this module):
// - No extended/targeted discovery, so only directly-connected peers
//   (reachable via the basic UDP Hello discovery) can become LDP peers.
// - No graceful restart, capabilities negotiation, or IGP synchronization.
// - No Generalized TTL Security Mechanism (GTSM).
// - No IPv6 support (RFC 7552); the whole module is IPv4-only.
// - No pseudowire (PW) signaling extensions.
//
// @author Vojta Janota
//
simple Ldp extends SimpleModule like IApp
{
    parameters:
        @class(Ldp);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string libTableModule;
        string tedModule;
        double holdTime @unit(s) = default(15s);
        double helloInterval @unit(s) = default(5s);
        double keepaliveTime @unit(s) = default(30s); // Session KeepAlive Time we propose during Initialization (RFC 5036 Section 3.5.3); RFC 5036 mandates no specific default, 30s is the conventional real-world value
        bool advertiseImplicitNull = default(true); // penultimate hop popping: advertise the implicit null label (RFC 3032) instead of allocating a local pop entry when acting as egress for a FEC
        string distributionMode @enum("du","dod") = default("du"); // RFC 5036 Section 2.6: label advertisement discipline. "du" (Downstream Unsolicited) proactively advertises Label Mappings to every OPERATIONAL peer and is the real-world/RFC-conventional default (a deliberate compliance choice); "dod" (Downstream on Demand) reproduces this model's original Label Request/Mapping request-response flow unchanged. Also wired into the Initialization message's A-bit (RFC 5036 Section 3.5.3): 0=DU, 1=DoD.
        string controlMode @enum("independent","ordered") = default("independent"); // RFC 5036 Section 2.6: label distribution control, meaningful only for transit FECs under distributionMode="du" (DoD is always effectively ordered along the FEC's next-hop chain). "independent" advertises a transit FEC's mapping as soon as this router is ready, without waiting for its own downstream mapping (the LIB swap entry is installed only once that mapping later arrives; an incoming labeled packet has nowhere to swap to in the meantime and is dropped -- the faithful, not buggy, independent-control transient). "ordered" advertises a transit FEC only once a downstream mapping is held or this router is egress for it.
        string retentionMode @enum("liberal","conservative") = default("liberal"); // RFC 5036 Section 2.6: label retention. "liberal" keeps a Label Mapping received from a peer that is not (or not yet) a FEC's next hop, enabling instant failover to it on a later next-hop change with no new Label Request; "conservative" explicitly releases such mappings (this model's original implicit behavior, now made explicit).
        bool loopDetection = default(false); // RFC 5036 Section 2.8: hop-count/path-vector loop detection for propagated Label Request and Label Mapping messages. Disabled by default -- loop detection is RFC-optional, and disabled is a fully compliant default (no shipped example relies on it, and this keeps this model's default behavior/fingerprint unchanged).
        int pathVectorLimit = default(32); // RFC 5036 Section 3.5.3 (Common Session Parameters TLV, PVLim field): maximum path-vector length (hop count) before a Label Request/Mapping is treated as having looped; only consulted when ~loopDetection is enabled.
        @display("i=block/control");
        @signal[sessionUp](type=long); // peer's router id (as a 32-bit integer) when an LDP session comes up
        @signal[sessionDown](type=long); // peer's router id (as a 32-bit integer) when an LDP session goes down
        @signal[fecBindingCount](type=long); // fecUp.size() + fecDown.size(), emitted whenever a label binding is added or removed
        @statistic[sessionUp](title="LDP sessions established"; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[sessionDown](title="LDP sessions torn down"; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[fecBindingCount](title="number of FEC label bindings"; record=last,vector; interpolationmode=none);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation

    gates:
        input socketIn @labels(TcpCommand/up,UdpCommand/up);
        output socketOut @labels(TcpCommand/down,UdpCommand/down);
}

