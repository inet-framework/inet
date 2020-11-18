package inet.showcases.wireless.ns3test;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee80211.bitlevel.Ieee80211LayeredDimensionalRadioMedium;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network Network
{
    @display("bgb=400,400");
    submodules:
        sourceHost: AdhocHost {
            @display("p=112,229");
        }
        destinationHost: AdhocHost {
            @display("p=322,229");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=341,71");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=330,143");
        }
}

network Dimensional
{
    @display("bgb=400,400");
    submodules:
        sourceHost: AdhocHost {
            @display("p=112,229");
        }
        destinationHost: AdhocHost {
            @display("p=322,229");
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            @display("p=341,71");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=330,143");
        }
}

network Symbol
{
    @display("bgb=400,400");
    submodules:
        sourceHost: AdhocHost {
            @display("p=112,229");
        }
        destinationHost: AdhocHost {
            @display("p=322,229");
        }
        radioMedium: Ieee80211LayeredDimensionalRadioMedium {
            @display("p=341,71");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=330,143");
        }
}
