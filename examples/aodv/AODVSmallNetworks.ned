//
// Copyright (C) 2014
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.aodv;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AodvRouter;
import inet.physicallayer.wireless.unitdisk.UnitDiskRadioMedium;


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
        sender: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,51");
        }
        intermediateNode: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=270,214");
        }
        receiver: AodvRouter {
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
        sender: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=284,51");
        }
        intermediateNodeA: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=203,206");
        }
        intermediateNodeB: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=386,206");
        }
        receiver: AodvRouter {
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
        sender: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=283,63");
        }
        intermediateNodeA: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=203,206");
        }
        intermediateNodeB: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=385,256");
        }
        intermediateNodeC: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=292,325");
        }
        receiver: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=292,454");
        }
    connections allowunconnected:
}
