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
// Explicit non-features (RFC 5036 mechanisms not modeled by this module):
// - No extended/targeted discovery, so only directly-connected peers
//   (reachable via the basic UDP Hello discovery) can become LDP peers.
// - Downstream-on-Demand with ordered control only; Downstream Unsolicited,
//   independent control, and liberal retention are not implemented, and
//   there is no parameter to select a mode.
// - No Address / Address Withdraw message support: such messages received
//   from a peer are logged and ignored rather than causing an error, and
//   the mapping from a peer to its next-hop interface is instead derived
//   from the local routing table. (~LdpAddress exists only as a wire-format
//   representation.)
// - No loop detection (hop count / path vector TLVs).
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

