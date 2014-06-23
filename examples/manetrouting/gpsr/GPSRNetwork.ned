//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.manetrouting.gpsr;

import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.gpsr.GPSRRouter;
import inet.physicallayer.ieee80211.Ieee80211RadioMedium;


//
// TODO
//
network GPSRNetworkRandom
{
    parameters:
        int numHosts;
    submodules:
        radioMedium: Ieee80211RadioMedium {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
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
        host[numHosts]: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
    connections allowunconnected:
}

network GPSRNetworkManual
{
    submodules:
        radioMedium: Ieee80211RadioMedium {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=50,100");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=50,150");
        }
        host0: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=37,300");
        }
        host1: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=723,300");
        }
        host2: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=185,300");
        }
        host3: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=345,300");
        }
        host4: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=345,157");
        }
        host5: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=346,26");
        }
        host6: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=446,26");
        }
        host7: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=346,446");
        }
        host8: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=345,572");
        }
        host9: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=446,572");
        }
        host10: GPSRRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070;p=642,100");
        }
}
