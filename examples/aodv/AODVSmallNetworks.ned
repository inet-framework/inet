//
// Copyright (C) 2014
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

package inet.examples.aodv;

import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AODVRouter;
import inet.physicallayer.idealradio.IdealRadioMedium;


network SimpleRREQ
{
    parameters:
        @display("bgb=525,437");
    submodules:
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=50,100");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=50,150");
        }
        lifecycleController: LifecycleController {
            parameters:
                @display("p=50,200");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=50,250");
        }
        sender: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,51");
        }
        intermediateNode: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,214");
        }
        receiver: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,377");
        }
    connections allowunconnected:
}

network SimpleRREQ2
{
    parameters:
        @display("bgb=961,662");
    submodules:
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=50,100");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=50,150");
        }
        lifecycleController: LifecycleController {
            parameters:
                @display("p=50,200");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=50,250");
        }
        sender: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=284,51");
        }
        intermediateNodeA: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=203,206");
        }
        intermediateNodeB: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=386,206");
        }
        receiver: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=284,378");
        }
    connections allowunconnected:
}

network ShortestPath
{
    parameters:
        @display("bgb=961,662");
    submodules:
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=50,100");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=50,150");
        }
        lifecycleController: LifecycleController {
            parameters:
                @display("p=50,200");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=50,250");
        }
        sender: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=283,63");
        }
        intermediateNodeA: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=203,206");
        }
        intermediateNodeB: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=385,256");
        }
        intermediateNodeC: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=292,325");
        }
        receiver: AODVRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=292,454");
        }
    connections allowunconnected:
}
