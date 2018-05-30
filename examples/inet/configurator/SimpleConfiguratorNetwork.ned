//
// Copyright (C) 2012 Opensim Ltd.
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

package inet.examples.inet.configurator;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.StandardHost;

network SimpleConfiguratorNetwork
{
    @display("bgb=450,300");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host11: StandardHost {
            parameters:
                @display("p=250,100");
        }
        host21: StandardHost {
            parameters:
                @display("p=400,100");
        }
        host12: StandardHost {
            parameters:
                @display("p=250,250");
        }
        host22: StandardHost {
            parameters:
                @display("p=400,250");
        }
    connections allowunconnected:
        host11.ethg++ <--> Eth1G <--> host21.ethg++;
        host11.ethg++ <--> Eth1G <--> host12.ethg++;
        host21.ethg++ <--> Eth1G <--> host22.ethg++;
        host12.ethg++ <--> Eth1G <--> host22.ethg++;
}
