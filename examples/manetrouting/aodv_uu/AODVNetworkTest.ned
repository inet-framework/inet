//
// Copyright (C) 2014 OpenSim Ltd.
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
//RK--------------RK
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.


package inet.examples.manetrouting.aodv_uu;

import inet.environment.common.PhysicalEnvironment;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AodvRouter;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.Eth10M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.node.inet.Router;
import inet.node.inet.AdhocHost;



network AODV_UU
{
    parameters:
        @display("bgb=40,25;bgg=1,1,grey95");


    submodules:
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=10,1;is=s");
        }
        
        physicalEnvironment: PhysicalEnvironment {
            @display("p=35,1");
        }
        
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            @display("p=30,1");
        }
        
        configurator: Ipv4NetworkConfigurator {
            @display("p=25,1");
        }
  
        
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=20,1;is=s");
        }
        
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=15,1;is=s");
        }
        
        
        Host1: AdhocHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=8.25,8");
        }  
        
        Host2: AdhocHost {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=8.25,16");
        }
        
        Router1: AdhocHost{ //Router {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=12,8");
        }

        Router2: AdhocHost{ //Router {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=12,14");
        }    

        connections:
        Router1.ethg++ <--> Eth10M <--> Router2.ethg++;

}
