//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

package inet.examples.diffserv.onedomain;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network DiffservNetwork
{
    parameters:
        double edgeDatarate @unit(bps);
        double coreDatarate @unit(bps);
    types:
        channel edgeline extends ThruputMeteringChannel
        {
            delay = 2ms;
            datarate = edgeDatarate;
            thruputDisplayFormat = "b B U";
        }
        channel coreline extends ThruputMeteringChannel
        {
            delay = 2ms;
            datarate = coreDatarate;
            thruputDisplayFormat = "b B U";
        }
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "b B U";
        }
    submodules:
        H1: StandardHost {
            @display("p=46,43");
        }
        H2: StandardHost {
            @display("p=46,143");
        }
        R4: Router {
            @display("p=473,108");
        }
        R2: Router {
            @display("p=237,178");
        }
        R1: Router {
            @display("p=156,263");
        }
        R3: Router {
            @display("p=390,178");
        }
        R5: Router {
            @display("p=473,263");
        }
        R0: Router {
            @display("p=156,108");
        }
        H5: StandardHost {
            @display("p=570,59");
        }
        H6: StandardHost {
            @display("p=570,150");
        }
        H7: StandardHost {
            @display("p=571,250");
        }
        H8: StandardHost {
            @display("p=571,350");
        }
        H3: StandardHost {
            @display("p=41,240");
        }
        H4: StandardHost {
            @display("p=42,346");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=300,24");
        }
    connections:
        H1.ethg++ <--> ethernetline <--> R0.ethg++;
        H2.ethg++ <--> ethernetline <--> R0.ethg++;
        H3.ethg++ <--> ethernetline <--> R1.ethg++;
        H4.ethg++ <--> ethernetline <--> R1.ethg++;
        H5.ethg++ <--> ethernetline <--> R4.ethg++;
        H6.ethg++ <--> ethernetline <--> R4.ethg++;
        H7.ethg++ <--> ethernetline <--> R5.ethg++;
        H8.ethg++ <--> ethernetline <--> R5.ethg++;
        R0.pppg++ <--> edgeline <--> R2.pppg++;
        R1.pppg++ <--> edgeline <--> R2.pppg++;
        R2.pppg++ <--> coreline <--> R3.pppg++;
        R4.pppg++ <--> coreline <--> R3.pppg++;
        R5.pppg++ <--> coreline <--> R3.pppg++;
}
