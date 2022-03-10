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
// IPv6 compatible node with MIPv6 support.
// Models a host with one wireless (802.11b) card in infrastructure mode, supports handovers and MIPv6 protocol.
//
// @see ~AccessPoint, ~WirelessHost, ~AdhocHost
//
module WirelessHost6 extends StandardHost6
{
    numWlanInterfaces = 1;
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = true; //is Mobile Node
    ipv6.mipv6support.isHomeAgent = false; //is Home Agent
    @display("i=device/wifilaptop");
}

