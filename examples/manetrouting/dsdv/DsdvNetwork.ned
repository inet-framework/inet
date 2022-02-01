//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.manetrouting.dsdv;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.dsdv.DsdvRouter;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


//
// TODO
//
network DsdvNetwork
{
    parameters:
        int numHosts;
        @display("bgb=650,650");
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
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
        host[numHosts]: DsdvRouter {
            parameters:
                dsdv.netmask = "255.255.0.0";
                @display("i=device/pocketpc_s;r=,,#707070");
        }
    connections allowunconnected:
}

