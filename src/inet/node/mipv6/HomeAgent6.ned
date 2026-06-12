//
// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund
// faqir.yousaf@uni-dortmund.de
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.node.mipv6;

import inet.node.ipv6.Router6;

//
// An IPv6 router that functions as a Home Agent in Mobile IPv6 networks.
// Extends the standard IPv6 router (Router6) with Mobile IPv6 support.
//
// The Home Agent is a specialized router that maintains information about mobile nodes'
// current locations. It intercepts packets destined for mobile nodes that are away from
// their home network and tunnels them to the mobile nodes' current care-of addresses.
//
// MIPv6 routers need faster Router Advertisements on wireless-facing interfaces
// to enable timely movement detection (RFC 3775 Section 7.5). Configure via
// ~Ipv6NetworkConfigurator XML, e.g.:
//
//   `<interface hosts='ha*' names='wlan*' maxRtrAdvInterval='1.5' minRtrAdvInterval='0.5'/>`
//
// @see ~MobileHost6, ~CorrespondentNode6
//
module HomeAgent6 extends Router6
{
    ipv6.hasMipv6 = true;
    ipv6.mipv6.isMobileNode = false; // Is Mobile Node
    ipv6.mipv6.isHomeAgent = true; // Is Home Agent
}
