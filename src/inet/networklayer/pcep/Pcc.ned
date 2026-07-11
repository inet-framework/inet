//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.pcep;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// PCC (Path Computation Client) (RFC 5440). Opens a TCP connection to a single,
// statically configured PCE (~Pce) on ~pcePort (4189 by default), and always plays
// the active (connecting) role -- unlike ~Ldp's Hello-driven active/passive
// election, PCEP has no discovery phase, so the PCC always initiates.
//
// Placement note: see ~Pce's class documentation for the inet.networklayer.pcep
// placement rationale (shared with this module).
//
// Phase 1 of this workstream (this module): session establishment only -- see
// ~Pce's class documentation for the full FSM/negotiation description, which
// applies symmetrically here (KeepAlive Time negotiated as the smaller of both
// sides' proposals; DeadTimer applied using the PEER's -- here, the PCE's --
// advertised value, not ~deadTimer).
//
// Reconnection: PCEP has no discovery/Hello phase to piggyback reconnection on
// (unlike ~Ldp, whose surviving Hello adjacency automatically re-establishes a lost
// session); instead, losing the session (TCP failure or DeadTimer expiry) schedules
// an explicit reconnection attempt after ~reconnectInterval.
//
// Phase 3 (RFC 8231 stateful delegation, Workstream F4): when ~RsvpTe's own
// "delegate" parameter is on, this Pcc reports every delegated ingress LSP RsvpTe
// manages to the PCE via reportLsp() (a PCRpt), and applies an incoming PCUpd by
// calling back into RsvpTe (via "rsvpteModule") to make-before-break re-route the
// named LSP onto the PCE-supplied ERO. If the PCEP session is lost while LSPs are
// still delegated, this Pcc waits ~stateTimeout (RFC 8231 Section 5.2) for the PCE
// to come back before telling RsvpTe to fall back to local (non-PCE) path
// computation for each of them -- see ~RsvpTe's own "delegate" parameter doc
// comment for the full picture; a plain stateless (Phase 1/2-only) deployment never
// sets "rsvpteModule" and never touches any of this.
//
simple Pcc extends SimpleModule like IApp
{
    parameters:
        @class(Pcc);
        string pceAddress; // address (module name or IP) of the PCE to connect to; resolved via L3AddressResolver at each connection attempt
        int pcePort = default(4189); // RFC 5440 Section 4.1: IANA-allocated PCEP port
        string localAddress = default(""); // optional: address (module name or IP) to bind the local TCP endpoint to, resolved via L3AddressResolver; "" (default) leaves the local address unspecified, letting the stack pick whichever outgoing interface's address routes toward the PCE (today's Phase 1 behavior, unaffected). Set this to this router's own stable, network-wide-routable identity (its routerId/loopback) when the PCE is more than one hop away in an IGP/TED-routed backbone: this model's own ~Ted/~LinkStateRouting (like a real IGP) only guarantees reachability to each router's OWN routerId, not to its individual per-link interface addresses -- so a PCC that lets an arbitrary outgoing-interface address become its TCP connection's source address may be unreachable for the PCE's return traffic (SYN-ACK etc.) even though the PCE itself is perfectly reachable.
        double keepaliveTime @unit(s) = default(30s); // KeepAlive Time this PCC proposes in its Open (RFC 5440 Section 7.3); RFC-conventional default
        double deadTimer @unit(s) = default(120s); // DeadTimer this PCC proposes in its Open (RFC 5440 Section 7.3); RFC-conventional default is 4x keepaliveTime
        double reconnectInterval @unit(s) = default(5s); // delay before retrying the TCP connection after the session is lost (see class documentation)
        string rsvpteModule = default(""); // Phase 3: module-path reference to the co-located ~RsvpTe this Pcc's delegated LSPs belong to (e.g. "^.rsvp"); "" (default) disables all Phase 3 behavior -- no PCUpd can be applied and stateTimeout fallback has nothing to act on
        double stateTimeout @unit(s) = default(60s); // Phase 3 (RFC 8231 Section 5.2): how long this PCC waits, after the PCEP session is lost, for the PCE to come back before telling RsvpTe to fall back to local computation for every LSP still delegated at that point; only armed while at least one LSP is actually delegated
        @display("i=block/control");
        @signal[sessionUp](type=long); // emitted (with this session's local SID) whenever the PCEP session reaches OPERATIONAL
        @signal[sessionDown](type=long); // emitted (with this session's local SID) whenever an OPERATIONAL PCEP session goes down
        @statistic[sessionUp](title="PCEP sessions established"; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[sessionDown](title="PCEP sessions torn down"; record=count,"vector(constant1)"; interpolationmode=none);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s); // extra time after the lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s); // timeout value for the lifecycle stop operation

    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
