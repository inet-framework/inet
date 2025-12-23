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


network VisualizationE extends VisualizationD
{
    submodules:
        pedestrianVideo: WirelessHost {
            parameters:
                @display("p=316.776,213.864;i=device/wifilaptop");
        }
        router0: Router {
            parameters:
                @display("p=485.616,265.32;i=abstract/router");
        }
        switch0: EtherSwitch {
            parameters:
                @display("p=414.864,358.584");
        }
        videoStreamServer: StandardHost {
            parameters:
                @display("p=485.616,501.696");
        }
        server1: StandardHost {
            @display("p=350.544,471.144");
        }
    connections:
        accessPoint0.ethg++ <--> Eth100M <--> router0.ethg++;
        switch0.ethg++ <--> Eth100M <--> router0.ethg++;
        videoStreamServer.ethg++ <--> Eth100M <--> switch0.ethg++;
        server1.ethg++ <--> Eth100M <--> switch0.ethg++;
}
