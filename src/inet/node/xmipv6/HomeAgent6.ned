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
// IPv6 router configured to operate as a Home Agent in a network supporting MIPv6.
//
module HomeAgent6 extends Router6
{
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = false; //is Mobile Node
    ipv6.mipv6support.isHomeAgent = true; //is Home Agent
}

