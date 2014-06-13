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

import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth10M;
import inet.node.inet.StandardHost;


network DHCPShutdownReboot
{
    submodules:
        scenarioManager: ScenarioManager {
            @display("p=73,59");
        }
        lifecycleController: LifecycleController {
            @display("p=73,185");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='server' address='192.168.1.1' netmask='255.255.255.0'/></config>");
                @display("p=73,116");
        }
        client: StandardHost {
            @display("p=205,124");
        }
        server: StandardHost {
            @display("p=553,124");
        }
    connections:
        client.ethg++ <--> Eth10M <--> server.ethg++;
}
