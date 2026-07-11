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
// Phase 1 of this workstream (this module): session establishment only -- Open/
// Keepalive exchange (RFC 5440 Sections 6.2-6.4, 7.3) bringing a session to
// OPERATIONAL, plus DeadTimer-based liveness. No path computation (PCReq/PCRep), no
// stateful delegation (PCRpt/PCUpd, ERO application) yet -- those are later phases
// of this workstream. ~tedModule is wired (an "omniscient" module-path reference,
// mirroring ~Ldp's own tedmod and ~Ted's own initializeTED()'s pre-existing
// topology-omniscience elsewhere in this codebase) but not yet consulted.
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
simple Pce extends SimpleModule like IApp
{
    parameters:
        @class(Pce);
        string tedModule; // module-path reference to this router's Ted (wired, not yet consulted this phase -- see class documentation)
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
