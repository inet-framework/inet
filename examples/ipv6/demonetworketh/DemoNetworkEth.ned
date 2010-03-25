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


package inet.examples.ipv6.demonetworketh;

import inet.examples.ethernet.lans.ethernetline;
import inet.networklayer.autorouting.FlatNetworkConfigurator6;
import inet.nodes.ipv6.Router6;
import inet.nodes.ipv6.StandardHost6;
import inet.util.TCPDump;
import ned.DatarateChannel;


network DemoNetworkEth
{
    parameters:
        int n;
    types:
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        configurator: FlatNetworkConfigurator6;
        r1: Router6;
        r2: Router6;
        cli[n]: StandardHost6;
        srv[n]: StandardHost6;
        linemonitor[n]: TCPDump;
    connections:
        for i=0..n-1 {
// FIXME adjust to the new TCPDump module
//            cli[i].ethg++ <--> ethernetline <--> linemonitor[i].a;
//            linemonitor[i].b <--> ethernetline <--> r1.ethg++;
            cli[i].ethg++ <--> ethernetline <--> r1.ethg++;
            srv[i].ethg++ <--> ethernetline <--> r2.ethg++;
        }
        r1.ethg++ <--> ethernetline <--> r2.ethg++;
}
