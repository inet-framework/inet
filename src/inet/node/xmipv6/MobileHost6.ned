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
// MIPv6 host with TCP, UDP layers and applications.
//
module MobileHost6 extends StandardHost6
{
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = true; //is Mobile Node
    ipv6.mipv6support.isHomeAgent = false; //is Home Agent
}

