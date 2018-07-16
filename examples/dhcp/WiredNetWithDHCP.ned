//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.examples.dhcp;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;


network WiredNetWithDHCP
{
    parameters:
        int numHosts;
    submodules:
        server: StandardHost {
            @display("p=400,225");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=450,350");
        }
        client[numHosts]: StandardHost {
            @display("p=150,100,ri,300,250");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='server' address='192.168.1.1' netmask='255.255.255.0' /></config>");
                @display("p=100,100;is=s");
        }
    connections:
        server.ethg++ <--> Eth10M <--> switch.ethg++;
        for i=0..numHosts-1 {
            client[i].ethg++ <--> Eth10M <--> switch.ethg++;
        }
}

