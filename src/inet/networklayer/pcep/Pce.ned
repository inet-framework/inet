//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.pcep;

import inet.common.SimpleModule;
import inet.applications.contract.IApp;

//
// PCE (Path Computation Element) server (RFC 5440). Listens on TCP port 4189 (see
// ~PcepCommon's PCEP_PORT) and accepts PCEP sessions from any number of PCCs
// (~Pcc).
//
// Placement note: lives under inet.networklayer.pcep, alongside the other
// TCP-based signaling protocols for the same MPLS/TE data plane (~Ldp, ~RsvpTe),
// rather than under inet.applications -- see this workstream's Phase 1 commit
// message.
//
// Phase 1 of this workstream: session establishment only -- Open/Keepalive exchange
// (RFC 5440 Sections 6.2-6.4, 7.3) bringing a session to OPERATIONAL, plus
// DeadTimer-based liveness. Phase 2 (this revision) adds stateless path computation
// (PCReq/PCRep, RFC 5440 Section 6.5/6.6): ~tedModule (wired since Phase 1 -- an
// "omniscient" module-path reference, mirroring ~Ldp's own tedmod and ~Ted's own
// initializeTED()'s pre-existing topology-omniscience elsewhere in this codebase) is
// now actually consulted, running the identical CSPF (Ted::calculateShortestPath())
// ~RsvpTe's own local computation uses (Workstream C6), rooted at the requesting
// PCC rather than at this node. No stateful delegation (PCRpt/PCUpd, ERO
// application) yet -- that is a later phase of this workstream.
//
// Session FSM (RFC 5440 Section 6.2, minimal subset -- mirrors the level of
// fidelity of ~Ldp's own Initialization/KeepAlive FSM, not the full RFC state
// machine): a PCC's TCP connection is accepted unconditionally (PCEP has no
// discovery phase, unlike LDP's Hello; any PCC that can reach this PCE on PCEP_PORT
// is accepted); once its Open arrives, this PCE replies with its own Open and a
// Keepalive; the session becomes OPERATIONAL once the PCC's Keepalive
// (acknowledging ours) is received. The negotiated KeepAlive Time used to schedule
// this PCE's own periodic Keepalives to a PCC is the smaller of the two sides'
// proposed values (RFC 5440 Section 7.3), symmetric to ~Ldp's own KeepAlive Time
// negotiation. The DeadTimer, in contrast, is NOT negotiated: each side instead
// arms its own liveness timer for the peer using the DeadTimer value THAT PEER
// advertised in ITS OWN Open (RFC 5440 Section 7.3: this value is the advertiser's
// own promise of how long it will itself wait, i.e. how long the receiver may
// safely go without sending); ~deadTimer is what this PCE itself advertises to
// PCCs, for them to apply when monitoring this PCE.
//
// A PCC has no standing peer identity here: once a PCC's session is lost (TCP
// failure or DeadTimer expiry), its entry is discarded outright; a reconnecting PCC
// is handled entirely afresh via a new incoming TCP connection.
//
// Phase 3 (RFC 8231 stateful delegation, Workstream F4): a PCC reports every LSP it
// delegates control of via a PCRpt (processPCRPT()); this Pce remembers each one
// (keyed by PLSP-ID, see the C++ class's DelegatedLsp) together with the CSPF
// inputs (source/dest/bandwidth/priority/affinity) it needs to recompute that LSP's
// path later. Reoptimization is triggered by an operator/~ScenarioManager
// "pce-reoptimize" command (see processCommand()) rather than automatically on
// every TED change -- a TED-change-triggered version was considered (per this
// workstream's plan) but rejected as harder to make deterministic in a module test
// (it would fire on an unpredictable number of intermediate/partial TED updates as
// LinkStateRouting floods a change through the network); an explicit, operator-timed
// command is both simpler to implement and reliably testable. The command names a
// PLSP-ID directly, or "all" to reoptimize every currently delegated LSP:
//
// <pre>
// <pce-reoptimize module="LSR4.app[0]" plspid="all"/>
// <pce-reoptimize module="LSR4.app[0]" plspid="100"/>
// </pre>
//
// Reoptimizing an LSP re-runs the identical CSPF call processPCREQ() uses
// (Ted::calculateShortestPath(), rooted at that LSP's own source) and, only if the
// result differs from the route that LSP is currently known to be using, sends a
// PCUpd carrying the new ERO to the owning PCC. No RFC 8231 Section 5.6 State
// Synchronization Sequence is implemented: if a PCC's session bounces and
// reconnects, this Pce does not automatically re-learn or re-verify which LSPs are
// still delegated to it -- a documented scope simplification for this phase.
//
simple Pce extends SimpleModule like IApp
{
    parameters:
        @class(Pce);
        string tedModule; // module-path reference to this router's Ted, consulted to answer PCReq/PCRep path computations (see class documentation)
        double keepaliveTime @unit(s) = default(30s); // KeepAlive Time this PCE proposes in its Open (RFC 5440 Section 7.3); RFC-conventional default
        double deadTimer @unit(s) = default(120s); // DeadTimer this PCE proposes in its Open (RFC 5440 Section 7.3); RFC-conventional default is 4x keepaliveTime
        @display("i=block/control");
        @signal[sessionUp](type=long); // emitted (with the session's local SID) whenever a PCEP session reaches OPERATIONAL
        @signal[sessionDown](type=long); // emitted (with the session's local SID) whenever an OPERATIONAL PCEP session goes down
        @statistic[sessionUp](title="PCEP sessions established"; record=count,"vector(constant1)"; interpolationmode=none);
        @statistic[sessionDown](title="PCEP sessions torn down"; record=count,"vector(constant1)"; interpolationmode=none);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s); // extra time after the lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s); // timeout value for the lifecycle stop operation

    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}
