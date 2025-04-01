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
// A wireless mobile node with IPv6 and Mobile IPv6 support.
// Combines wireless connectivity with mobility management capabilities, allowing
// it to maintain network connections while moving between different wireless networks.
//
// This module extends StandardHost6 by adding a wireless interface and Mobile IPv6
// functionality. It is designed to work in infrastructure mode, connecting to access
// points and performing handovers when moving between different wireless networks.
//
// @see ~MobileHost6
//
module WirelessHost6 extends StandardHost6
{
    numWlanInterfaces = 1;
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = true; // Is a Mobile Node
    ipv6.mipv6support.isHomeAgent = false; // Is a Home Agent
    @display("i=device/wifilaptop");
}
