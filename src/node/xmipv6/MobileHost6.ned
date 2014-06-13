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


package inet.nodes.xmipv6;

import inet.nodes.ipv6.StandardHost6;


//
// MIPv6 host with TCP, UDP layers and applications.
//
module MobileHost6 extends StandardHost6
{
    networkLayer.xMIPv6Support = true;
    networkLayer.mipv6support.isMobileNode = true; //is Mobile Node
    networkLayer.mipv6support.isHomeAgent = false; //is Home Agent
}

