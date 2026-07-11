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
simple Pcc extends SimpleModule like IApp
{
    parameters:
        @class(Pcc);
        string pceAddress; // address (module name or IP) of the PCE to connect to; resolved via L3AddressResolver at each connection attempt
        int pcePort = default(4189); // RFC 5440 Section 4.1: IANA-allocated PCEP port
        double keepaliveTime @unit(s) = default(30s); // KeepAlive Time this PCC proposes in its Open (RFC 5440 Section 7.3); RFC-conventional default
        double deadTimer @unit(s) = default(120s); // DeadTimer this PCC proposes in its Open (RFC 5440 Section 7.3); RFC-conventional default is 4x keepaliveTime
        double reconnectInterval @unit(s) = default(5s); // delay before retrying the TCP connection after the session is lost (see class documentation)
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
