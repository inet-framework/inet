package inet.examples.wireless.power;

import inet.common.lifecycle.LifecycleController;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network PowerNetwork
{
    parameters:
        int numHost;
    submodules:
        lifecycleController: LifecycleController {
            @display("p=50,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=50,100");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=50,150");
        }
        host[numHost]: AdhocHost {
            @display("p=150,300");
        }
}

