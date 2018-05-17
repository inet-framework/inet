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
                @display("p=50,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=50,100");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=50,150");
        }
        host[numHosts]: AodvRouter {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
}
