package inet.examples.wireless.power;

import inet.common.lifecycle.LifecycleController;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;

network PowerNetwork
{
    parameters:
        int numHost;
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,250");
        }
        lifecycleController: LifecycleController {
            parameters:
                @display("p=100,350");
        }
        host[numHost]: AdhocHost {
            parameters:
            @display("p=150,300");
        }
}
