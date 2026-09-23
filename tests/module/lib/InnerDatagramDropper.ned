//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.test.moduletest.lib;

//
// Test helper: a netfilter PREROUTING hook on an IPv6 tunnel exit point that
// drops every UDP datagram, the way a firewall or an IPsec discard policy would.
// A tunnel packet carries IPv6 as its next header, so it passes, and the hook
// sees the inner datagram when the exit point re-processes it.
//
simple InnerDatagramDropper
{
    parameters:
        string networkProtocolModule; // path of the Ipv6 module to hook into
        @display("i=block/control");
}
