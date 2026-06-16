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
        @display("i=block/cogwheel");

    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);
}
