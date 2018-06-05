//
// Copyright (C) 2008 Irene Ruengeler
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

package inet.examples.emulation.extclient;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;


network ExtClient
{
    parameters:
        double testTimeout @unit(s) = default(0s);
        bool testing = default(false);
        @display("bgb=310,255");
    submodules:
        peer: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "cli1d.mrt";
                numExtInterfaces = 1;
                @display("p=29,90;i=old/laptop3");
        }
        etherSwitch: EtherSwitch {
            @display("p=155,123");
        }
        peer2: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "cli1d.mrt";
                numExtInterfaces = 1;
                @display("p=249,70;i=old/laptop3");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=75,188");
        }
    connections allowunconnected:
        peer.ethg++ <--> Eth1G <--> etherSwitch.ethg++;
        etherSwitch.ethg++ <--> Eth1G { per = 0.0; }<--> peer2.ethg++;
}



