//
// Copyright (C) 2012 OpenSim Ltd.
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

package inet.examples.internetcloud.cloudandrouters;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.internetcloud.InternetCloud;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import inet.visualizer.scene.NetworkNodeVisualizer;
import ned.DatarateChannel;


network CloudAndRouters
{
    parameters:
        int numSenderNets;
        int numHostsPerNet @display("bgb=694,791");
        @display("bgb=672,745");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1ms;
            datarate = 1000Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,401;is=s");
        }

        srouter[numSenderNets]: Router {
            @display("p=250,282,row,${numHostsPerNet}00");
        }
        sender[numSenderNets*numHostsPerNet]: StandardHost {
            @display("p=250,100,row,100");
        }
        recip: StandardHost {
            @display("p=250,700");
        }
        internet: InternetCloud {
            @display("p=250,400;is=vl");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=100,158");
        }
        etherSwitch[numSenderNets]: EtherSwitch {
            @display("p=250,232,row,${numHostsPerNet}00");
        }
    connections:
        for i=0..numSenderNets-1 {
            srouter[i].pppg++ <--> C <--> internet.pppg++;
            etherSwitch[i].ethg++ <--> Eth1G <--> srouter[i].ethg++;
        }
        for i=0..(numSenderNets*numHostsPerNet)-1 {
            sender[i].ethg++ <--> Eth1G <--> etherSwitch[int(floor(i/numHostsPerNet))].ethg++;
        }

        internet.pppg++ <--> C <--> recip.pppg++;

}

