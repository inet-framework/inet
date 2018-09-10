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

package inet.showcases.general.scenarioScripting;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.inet.Router;
import inet.node.wireless.AccessPoint;
import inet.node.ethernet.Eth10M;
import inet.networklayer.configurator.ipv4.HostAutoConfigurator;

import inet.node.ethernet.EtherSwitch;

module DynamicHost extends AdhocHost
{
    parameters:
        ipv4.configurator.networkConfiguratorModule = "";
        @display("i=misc/smartphone");
    submodules:
        autoConfigurator: HostAutoConfigurator {
            @display("is=s;p=125,625");
        }
}


network DynamicShowcase
{
    @display("bgb=600,400");
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("is=s;p=100,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("is=s;p=100,150");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("is=s;p=100,250");
        }
        destinationNode: DynamicHost {
            parameters:
                @display("p=350,200");
        }
}

network DynamicShowcase2
{
    @display("bgb=800,400");
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("is=s;p=50,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("is=s;p=50,150");
        }
        scenarioManager: ScenarioManager {
            parameters:
                @display("is=s;p=50,250");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("is=s;p=50,350");
        }
        sourceNode1: StandardHost {
            parameters:
                @display("p=200,100");
        }
        sourceNode2: StandardHost {
            parameters:
                @display("p=200,300");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=300,200");
        }
        edgeRouter: Router {
            parameters:
                @display("p=400,200");
        }
        backboneRouter: Router {
            parameters:
                @display("p=500,200");
        }
        destinationNode: StandardHost {
            parameters:
                @display("p=700,200");
        }
	connections:
	    sourceNode1.ethg++ <--> Eth10M <--> switch.ethg++;
	    sourceNode2.ethg++ <--> Eth10M <--> switch.ethg++;
	    switch.ethg++ <--> Eth10M <--> edgeRouter.ethg++;
	    edgeRouter.pppg++ <--> Eth10M <--> backboneRouter.pppg++;
	    backboneRouter.ethg++ <--> Eth10M <--> destinationNode.ethg++;
}