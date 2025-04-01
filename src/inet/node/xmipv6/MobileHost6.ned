//
// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund
// faqir.yousaf@uni-dortmund.de
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.node.xmipv6;

import inet.node.ipv6.StandardHost6;

//
// A host with Mobile IPv6 support. Extends the standard
// IPv6 host (StandardHost6) with mobility management capabilities.
//
// Mobile IPv6 allows a node to remain reachable while moving around in the IPv6
// Internet. The mobile node uses two IP addresses: a home address that is stable
// and a care-of address that changes at each new point of attachment.
//
// @see ~HomeAgent6, ~CorrespondentNode6
//
module MobileHost6 extends StandardHost6
{
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = true; // Is a Mobile Node
    ipv6.mipv6support.isHomeAgent = false; // Is a Home Agent
}
