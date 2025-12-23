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

package inet.tutorials.visualization;

import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;


network VisualizationH extends VisualizationG
{
    submodules:
        car: WirelessHost {
            @display("p=215,122");
        }
        accessPoint2: AccessPoint {
            @display("p=215,233");
        }
        router2: Router {
            @display("p=316,234");
        }
        accessPoint3: AccessPoint {
            @display("p=215,298");
        }
        tcpHost[2]: StandardHost {
            @display("p=650,431");
        }
        switch1: EtherSwitch {
            @display("p=660,250");
        }
        router3: Router {
            @display("p=316,299");
        }
    connections:
        router0.pppg++ <--> Eth100M <--> router2.pppg++;
        router2.ethg++ <--> Eth100M <--> accessPoint2.ethg++;
        switch0.ethg++ <--> Eth100M <--> tcpHost[0].ethg++;
        switch1.ethg++ <--> Eth100M <--> tcpHost[1].ethg++;
        router1.ethg++ <--> Eth100M <--> switch1.ethg++;
        router0.pppg++ <--> Eth100M <--> router3.pppg++;
        accessPoint3.ethg++ <--> Eth100M <--> router3.ethg++;
}
