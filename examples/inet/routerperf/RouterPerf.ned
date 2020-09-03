//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

package inet.examples.inet.routerperf;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;


network RouterPerfNetwork
{
    parameters:
        int nodeNo;
    types:
        channel C extends ThruputMeteringChannel
        {
            thruputDisplayFormat = "u";
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        sender[nodeNo]: BurstHost {
            parameters:
                @display("p=250,300,row,150");
        }
        recip[nodeNo]: BurstHost {
            parameters:
                @display("p=250,100,row,150");
        }
        router: Router {
            parameters:
                @display("p=400,200");
        }
    connections:
        for i=0..nodeNo-1 {
            sender[i].pppg++ <--> C {  datarate = 1Mbps; } <--> router.pppg++;
            recip[i].pppg++ <--> C {  datarate = 1Mbps; } <--> router.pppg++;
        }
}

