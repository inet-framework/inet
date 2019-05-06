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

package inet.examples.loadng;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AodvRouter;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.examples.loadng.LoadNgHost;

network SimpleRREQ
{
    parameters:
    submodules:
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=100,100;is=s");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=100,300;is=s");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=100,400;is=s");
        }
        sender: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,51");
        }
        intermediateNode: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,214");
        }
        receiver: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,377");
        }
    connections allowunconnected:
}

network SimpleRREQ2
{
    parameters:
    submodules:
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=100,100;is=s");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=100,300;is=s");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=100,400;is=s");
        }
        sender: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=284,51");
        }
        intermediateNodeA: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=203,206");
        }
        intermediateNodeB: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=386,206");
        }
        receiver: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=284,378");
        }
    connections allowunconnected:
}

network ShortestPath
{
    parameters:
        @display("bgb=650,550");
    submodules:
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
                addStaticRoutes = false;
                addSubnetRoutes = false;
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=100,100;is=s");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=100,300;is=s");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=100,400;is=s");
        }
        sender: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=283,63");
        }
        intermediateNodeA: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=203,206");
        }
        intermediateNodeB: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=385,256");
        }
        intermediateNodeC: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=292,325");
        }
        receiver: LoadNgHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=292,454");
        }
    connections allowunconnected:
}
