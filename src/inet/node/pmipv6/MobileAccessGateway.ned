//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.pmipv6;

import inet.node.ipv6.Router6;

//
// An IPv6 access router that acts as a Mobile Access Gateway (MAG) in a Proxy
// Mobile IPv6 (RFC 5213) domain. It detects mobile nodes attaching to and
// detaching from its access links, sends Proxy Binding Updates to the Local
// Mobility Anchor on their behalf, sets up the tunnel to the anchor, and
// advertises each mobile node's home network prefix on the access link so the
// (unmodified) mobile node keeps its address as it roams between gateways.
//
// Its wireless interfaces are configured as IEEE 802.11 access points by default,
// so mobile nodes (plain wireless IPv6 hosts) associate with it; the access
// technology is otherwise unconstrained.
//
// @see ~LocalMobilityAnchor, ~Pmipv6
//
module MobileAccessGateway extends Router6
{
    ipv6.hasPmipv6 = true;
    ipv6.pmipv6.isMobileAccessGateway = true;
    ipv6.pmipv6.isLocalMobilityAnchor = false;
    // wireless access interfaces act as 802.11 access points (mobile nodes associate with them)
    wlan[*].mgmt.typename = default("Ieee80211MgmtAp");
    wlan[*].agent.typename = default("");
}
