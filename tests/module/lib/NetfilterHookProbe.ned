//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.test.moduletest.lib;

//
// Test-only netfilter hook. Registers on the network-protocol module given by
// `networkProtocolModule` (e.g. "^.ipv4.ip" or "^.ipv6.ipv6") and returns the
// configured verdict for each of the five netfilter hooks. On QUEUE it defers the
// datagram by `reinjectDelay` and then reinjects it, exercising the reinject path.
//
simple NetfilterHookProbe
{
    parameters:
        string networkProtocolModule; // path of the Ipv4/Ipv6 module to hook into
        string preRouting @enum("ACCEPT","DROP","QUEUE") = default("ACCEPT");
        string localIn @enum("ACCEPT","DROP","QUEUE") = default("ACCEPT");
        string forward @enum("ACCEPT","DROP","QUEUE") = default("ACCEPT");
        string postRouting @enum("ACCEPT","DROP","QUEUE") = default("ACCEPT");
        string localOut @enum("ACCEPT","DROP","QUEUE") = default("ACCEPT");
        double reinjectDelay @unit(s) = default(1ms);
        @display("i=block/control");
}
