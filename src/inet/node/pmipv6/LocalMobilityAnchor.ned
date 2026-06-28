//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.node.pmipv6;

import inet.node.ipv6.Router6;

//
// An IPv6 router that acts as the Local Mobility Anchor (LMA) in a Proxy Mobile
// IPv6 (RFC 5213) domain. It is the topological anchor for the mobile nodes'
// home network prefixes: it holds the binding cache, terminates the bidirectional
// tunnels to the Mobile Access Gateways, and routes each mobile node's home
// network prefix into the tunnel toward its currently serving gateway.
//
// @see ~MobileAccessGateway, ~Pmipv6
//
module LocalMobilityAnchor extends Router6
{
    ipv6.hasPmipv6 = true;
    ipv6.pmipv6.isLocalMobilityAnchor = true;
    ipv6.pmipv6.isMobileAccessGateway = false;
}
