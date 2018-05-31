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

package inet.examples.wireless.video.videoStreaming.simulations;

import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AodvRouter;
import inet.physicallayer.common.packetlevel.RadioMedium;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.node.inet.INetworkNode;

import inet.environment.common.PhysicalEnvironment;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.applications.contract.IApp;


//
// TODO auto-generated type
//
network Net
{
    parameters:
        //int numHosts;
        string hostType = default("WirelessHost");
        int numHosts;
        string mediumType = default("UnitDiskRadioMedium");
        @display("bgb=1000,700");

        //@figure[rcvdPkText](type=indicatorText; pos=380,20; anchor=w; font=,18; textFormat="packets received: %g"; initialValue=0);
        //@statistic[packetReceived](source=hostB.app[0].packetReceived; record=figure(count); targetFigure=rcvdPkText);

    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=909.56805,57.728");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=909.56805,133.76001");
        }
        radioMedium: <mediumType> like IRadioMedium {
            @display("p=909.56805,208.384");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=909.56805,357.63202");
        }
        hostA: <hostType> like INetworkNode {
            @display("i=device/pocketpc_s;p=691.328,388.608");
        }
        hostB: <hostType> like INetworkNode {
            @display("i=device/pocketpc_s;p=359.04,388.608");
        }
        hostR[numHosts]: <hostType> like INetworkNode {
            @display("i=device/pocketpc_s;r=,,#707070;");
        }
    connections allowunconnected:
}

//        radioMedium: UnitDiskRadioMedium {
//            @display("p=89,32");
//        }

