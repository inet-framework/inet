//
// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund
// faqir.yousaf@uni-dortmund.de
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.node.xmipv6;

import inet.node.ipv6.Router6;

//
// An IPv6 router that functions as a Home Agent in Mobile IPv6 networks.
// Extends the standard IPv6 router (Router6) with Mobile IPv6 support.
//
// The Home Agent is a specialized router that maintains information about mobile nodes'
// current locations. It intercepts packets destined for mobile nodes that are away from
// their home network and tunnels them to the mobile nodes' current care-of addresses.
//
// @see ~MobileHost6, ~CorrespondentNode6
//
module HomeAgent6 extends Router6
{
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = false; // Is Mobile Node
    ipv6.mipv6support.isHomeAgent = true; // Is Home Agent
}
