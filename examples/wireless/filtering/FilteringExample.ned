//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.wireless.filtering;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.aodv.AodvRouter;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;

network FilteringExampleNetwork
{
    parameters:
        int numHosts;
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,300;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        host[numHosts]: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
}
