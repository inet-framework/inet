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
// An IPv6 host that can communicate directly with
// mobile nodes in a Mobile IPv6 network. Extends the standard IPv6 host with
// Mobile IPv6 support for route optimization.
//
// In Mobile IPv6, a correspondent node is any node that communicates with mobile
// nodes. It maintains a binding cache that stores mappings between mobile nodes'
// home addresses and care-of addresses, allowing for direct communication without
// going through the home agent (route optimization).
//
// @see ~MobileHost6, ~HomeAgent6
//
module CorrespondentNode6 extends StandardHost6
{
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = false; // Is a Mobile Node
    ipv6.mipv6support.isHomeAgent = false; // Is a Home Agent
}
