//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.examples.seaport;

import inet.networks.base.WirelessNetworkBase;
import inet.node.ethernet.Eth1G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.routing.gpsr.Gpsr;

module GpsrStandardHost extends StandardHost
{
    submodules:
        gpsr: Gpsr;
    connections:
        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}

module GpsrRouter extends Router
{
    submodules:
        gpsr: Gpsr;
    connections:
        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}

module GpsrWirelessHost extends WirelessHost
{
    submodules:
        gpsr: Gpsr;
    connections:
        gpsr.ipOut --> tn.in++;
        gpsr.ipIn <-- tn.out++;
}

network SeaportNetwork extends WirelessNetworkBase
{
    parameters:
        int numShips;
        @display("bgi=misc/seaport;bgb=1800,1000");
    submodules:
        ap1: AccessPoint {
            @display("i=device/antennatower;p=1243,92");
        }
        ap2: AccessPoint {
            @display("i=device/antennatower;p=1739,220");
        }
        ap3: AccessPoint {
            @display("i=device/antennatower;p=597,269");
        }
        ap4: AccessPoint {
            @display("i=device/antennatower;p=1138,484");
        }
        ap5: AccessPoint {
            @display("i=device/antennatower;p=1680,663");
        }
        ap6: AccessPoint {
            @display("i=device/antennatower;p=505,907");
        }
        router1: GpsrRouter {
            @display("p=1530,179");
        }
        router2: GpsrRouter {
            @display("p=988,742");
        }
        router3: GpsrRouter {
            @display("p=1510,742");
        }
        server: GpsrStandardHost {
            @display("p=1586,886");
        }
        ship[numShips]: GpsrWirelessHost {
            @display("i=misc/transparent");
        }
    connections:
        router1.ethg++ <--> Eth1G <--> ap1.ethg++;
        router1.ethg++ <--> Eth1G <--> ap2.ethg++;
        router2.ethg++ <--> Eth1G <--> ap3.ethg++;
        router2.ethg++ <--> Eth1G <--> ap4.ethg++;
        router2.ethg++ <--> Eth1G <--> ap6.ethg++;
        router3.ethg++ <--> Eth1G <--> ap5.ethg++;
        router3.ethg++ <--> Eth1G <--> server.ethg++;
        router3.ethg++ <--> Eth1G <--> router1.ethg++;
        router3.ethg++ <--> Eth1G <--> router2.ethg++;
}
