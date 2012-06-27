//
// Copyright (C) 2012 OpenSim Ltd
// @author: Zoltan Bojthe
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

package inet.examples.dhcp.eth;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.StandardHost;
import inet.util.ThruputMeteringChannel;


network WiredNetWithDHCP
{
    parameters:
        int numHosts;
    types:
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
            thruputDisplayFormat = "u";
        }
    submodules:
        srv: StandardHost;
        switch: EtherSwitch {
            parameters:
                @display("p=186,250");
        }
        cli[numHosts]: StandardHost;
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='srv' address='192.168.1.1' netmask='255.255.255.0' /></config>");
        }
    connections:
        srv.ethg++ <--> ethernetline <--> switch.ethg++;
        for i=0..numHosts-1 {
            cli[i].ethg++ <--> ethernetline <--> switch.ethg++;
        }
}

