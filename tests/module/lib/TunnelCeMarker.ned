//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.test.moduletest.lib;

//
// Test helper: a netfilter FORWARD hook that sets Congestion Experienced on the
// outer header of every IPv6-in-IPv6 datagram carrying UDP that the node
// forwards, the way an ECN-marking router inside the tunnel would. It ignores
// the outer ECN field on purpose, so it also marks a Not-ECT outer header.
//
simple TunnelCeMarker
{
    parameters:
        string networkProtocolModule; // path of the Ipv6 module to hook into
        @display("i=block/control");
}
