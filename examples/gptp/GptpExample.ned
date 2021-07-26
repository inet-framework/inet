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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.//

package inet.examples.gptp;

import inet.common.scenario.ScenarioManager;
import inet.linklayer.configurator.MacAddressTableConfigurator;
import inet.networks.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;

network GptpTreeExample extends WiredNetworkBase
{
    submodules:
        grandMaster1: StandardHost {
            @display("p=500,150");
        }
        grandMaster2: StandardHost {
            @display("p=700,150");
        }
        bridge1: EthernetSwitch {
            @display("p=500,300");
        }
        bridge2: EthernetSwitch {
            @display("p=700,300");
        }
        endStation1: StandardHost {
            @display("p=300,450");
        }
        endStation2: StandardHost {
            @display("p=500,450");
        }
        endStation3: StandardHost {
            @display("p=700,450");
        }
        endStation4: StandardHost {
            @display("p=900,450");
        }
    connections:
        grandMaster1.ethg++ <--> Eth100M <--> bridge1.ethg++;
        grandMaster2.ethg++ <--> Eth100M <--> bridge2.ethg++;
        bridge1.ethg++ <--> Eth100M <--> bridge2.ethg++;
        bridge1.ethg++ <--> Eth100M <--> endStation1.ethg++;
        bridge1.ethg++ <--> Eth100M <--> endStation2.ethg++;
        bridge2.ethg++ <--> Eth100M <--> endStation3.ethg++;
        bridge2.ethg++ <--> Eth100M <--> endStation4.ethg++;
}

network GptpRingExample extends WiredNetworkBase
{
    submodules:
        macAddressTableConfigurator: MacAddressTableConfigurator {
            @display("p=100,400;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,500;is=s");
        }
        grandMaster1: StandardHost {
            @display("p=400,150");
        }
        grandMaster2: StandardHost {
            @display("p=800,600");
        }
        bridge1: EthernetSwitch {
            @display("p=600,150");
        }
        bridge2: EthernetSwitch {
            @display("p=400,300");
        }
        bridge3: EthernetSwitch {
            @display("p=400,450");
        }
        bridge4: EthernetSwitch {
            @display("p=600,600");
        }
        bridge5: EthernetSwitch {
            @display("p=800,450");
        }
        bridge6: EthernetSwitch {
            @display("p=800,300");
        }
        endStation1: StandardHost {
            @display("p=200,300");
        }
        endStation2: StandardHost {
            @display("p=200,450");
        }
        endStation3: StandardHost {
            @display("p=1000,450");
        }
        endStation4: StandardHost {
            @display("p=1000,300");
        }
    connections:
        grandMaster1.ethg++ <--> Eth100M <--> bridge1.ethg++;
        grandMaster2.ethg++ <--> Eth100M <--> bridge4.ethg++;
        bridge1.ethg++ <--> Eth100M <--> bridge2.ethg++;
        bridge2.ethg++ <--> Eth100M <--> bridge3.ethg++;
        bridge3.ethg++ <--> Eth100M <--> bridge4.ethg++;
        bridge4.ethg++ <--> Eth100M <--> bridge5.ethg++;
        bridge5.ethg++ <--> Eth100M <--> bridge6.ethg++;
        bridge6.ethg++ <--> Eth100M <--> bridge1.ethg++;
        bridge2.ethg++ <--> Eth100M <--> endStation1.ethg++;
        bridge3.ethg++ <--> Eth100M <--> endStation2.ethg++;
        bridge5.ethg++ <--> Eth100M <--> endStation3.ethg++;
        bridge6.ethg++ <--> Eth100M <--> endStation4.ethg++;
}
