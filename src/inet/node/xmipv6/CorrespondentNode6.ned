//
// Copyright (C) 2007 Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund
// faqir.yousaf@uni-dortmund.de
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.node.xmipv6;

import inet.node.ipv6.StandardHost6;

//
// An IPv6 host with MIPv6 support and contains a Binding Cache which gets updated with every BU received.
//
module CorrespondentNode6 extends StandardHost6
{
    ipv6.xMIPv6Support = true;
    ipv6.mipv6support.isMobileNode = false; //is Mobile Node
    ipv6.mipv6support.isHomeAgent = false; //is Home Agent
}

