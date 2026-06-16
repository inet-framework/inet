//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.icmpv6;

import inet.common.SimpleModule;

//
// Implements MLDv1 (RFC 2710): Multicast Listener Discovery for IPv6.
// Handles both the host listener role (joining/leaving groups) and the
// router querier role. This is the IPv6 equivalent of ~Igmpv2.
//
// In this phase the module is a skeleton: it registers ~Protocol::mld with
// the dispatcher and can send/receive MLD messages. State machines are
// implemented in Phases 3 and 4.
//
// @see ~IMld, ~Igmpv2
//
simple Mldv1 extends SimpleModule like IMld
{
    parameters:
        @class(Mldv1);
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        string checksumMode @enum("declared","computed") = default("declared");
        bool enabled = default(true);
        // Test-only: when true, schedules a single MLD Report send at t=0.5s
        // so the MLD_smoke.test can verify end-to-end dispatcher flow without
        // a real state machine. MUST remain default(false) so it is inert for
        // all production scenarios and other tests.
        bool sendTestMessage = default(false);
        // Test-only: when true, injects one General MLD Query (type 130, maxRespDelay=1000ms,
        // multicastAddress=::) on the first multicast-capable interface at t=2s.
        // This lets MLD_host_basic.test exercise the HOST-02 (Delaying Listener) path
        // without a real router querier (Phase 4). MUST remain default(false).
        bool sendTestQuery = default(false);
        // RFC 2710 §7.2 Unsolicited Report Interval: time between unsolicited
        // Multicast Listener Reports sent on join (default 10s per RFC 2710).
        double unsolicitedReportInterval @unit(s) = default(10s);
        @display("i=block/cogwheel");

    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);
}
