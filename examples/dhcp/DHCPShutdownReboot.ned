//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.examples.dhcp;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.inet.StandardHost;
import inet.visualizer.integrated.IntegratedVisualizer;


network DHCPShutdownReboot
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='server' address='192.168.1.1' netmask='255.255.255.0'/></config>");
                @display("p=100,125;is=s");
        }
        visualizer: IntegratedVisualizer {
            @display("p=100,50;is=s");
        }

        client: StandardHost {
            @display("p=250,150");
        }
        server: StandardHost {
            @display("p=600,150");
        }
    connections:
        client.ethg++ <--> Eth10M <--> server.ethg++;
}
