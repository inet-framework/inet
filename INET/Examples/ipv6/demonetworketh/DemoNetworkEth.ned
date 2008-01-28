//
// Copyright (C) 2005 Wei Yang, Ng
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


package org.omnetpp.inet.examples.IPv6.DemoNetworkEth;

import ned.cBasicChannel;
import org.omnetpp.inet.Network.AutoRouting.FlatNetworkConfigurator6;
import org.omnetpp.inet.Nodes.IPv6.Router6;


network DemoNetworkEth
{
    parameters:
        int n;
    types:
        channel ethernetline extends cBasicChannel
        {
            delay = 0.1us;
        }
    submodules:
        configurator: FlatNetworkConfigurator6 {
            parameters:
                moduleTypes = "Router6 StandardHost6 TCPDump";
                nonIPModuleTypes = "EtherHub TCPDump";
        }
        r1: Router6;
        r2: Router6;
}
