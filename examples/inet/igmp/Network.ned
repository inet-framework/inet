//
// Copyright (C) 2013 Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.examples.inet.igmp;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


//
// TODO documentation
//
network Network
{
    submodules:
        Alice: StandardHost {
            @display("p=76,62");
        }
        FITgate1: Router {
            @display("p=314,58");
        }
        FITgate2: Router {
            @display("p=314,168");
        }
        Bob: StandardHost {
            @display("p=76,172");
        }
        Carlos: StandardHost {
            @display("p=314,257");
        }
        etherSwitch: EtherSwitch {
            @display("p=203,124");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=195,25");
            config = xmldoc("config.xml");
        }
    connections allowunconnected:
        Alice.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        Bob.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        FITgate1.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        FITgate2.ethg++ <--> Eth100M <--> etherSwitch.ethg++;
        Carlos.ethg++ <--> Eth100M <--> FITgate2.ethg++;
}

