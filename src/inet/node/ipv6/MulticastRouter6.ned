//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.ipv6;

//
// An IPv6 router with multicast routing capabilities. It is the IPv6 counterpart
// of ~MulticastRouter: it extends the IPv6 ~Router6 node (no IPv4 stack;
// routingTableModule already points at the IPv6 routing table), enables
// Protocol Independent Multicast (PIM) and multicast forwarding, and binds the
// whole PIM compound (both PIM-DM and PIM-SM) to the IPv6 stack via
// pim.addressFamily = "ipv6".
//
// @see ~MulticastRouter
//
module MulticastRouter6 extends Router6
{
    parameters:
        hasPim = default(true);
        multicastForwarding = true;
        pim.addressFamily = default("ipv6");
}
