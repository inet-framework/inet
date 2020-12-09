package inet.showcases.wireless.analogmodel;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.ManetRouter;
import inet.physicallayer.wireless.apsk.bitlevel.ApskLayeredScalarRadioMedium;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.physicallayer.wireless.noise.NoiseSource;
import inet.physicallayer.wireless.unitdisk.UnitDiskRadioMedium;
import inet.visualizer.common.IntegratedMultiVisualizer;
import inet.visualizer.common.IntegratedVisualizer;

network AnalogModelShowcaseRoutingNetwork
{
    parameters:
        @display("bgb=400,400");
        int numHosts;

    submodules:
        source: ManetRouter {
            @display("p=104,335");
        }
        node[numHosts]: ManetRouter {
            @display("p=104,226");
        }
        destination: ManetRouter {
            @display("p=292,80");
        }
        radioMedium: UnitDiskRadioMedium {
            @display("p=38,28");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=38.461544,66.863914");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=38.461544,106.50889");
        }
}

network AnalogModelShowcaseDistanceNetwork
{
    parameters:
        @display("bgb=901.5187,376.65");
    submodules:
        visualizer: IntegratedMultiVisualizer {
            parameters:
                @display("p=64.256,62.247997");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=64.256,180.72");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=62.247997,287.14398");
        }
        destination: AdhocHost {
            parameters:
                @display("p=500,179.60625");
        }
        source: AdhocHost {
            @display("p=363.448,180.72");
        }
}

network AnalogModelShowcaseDistanceNetwork_
{
    parameters:
        @display("bgb=901.5187,376.65");
    submodules:
        visualizer: IntegratedMultiVisualizer {
            parameters:
                @display("p=64.256,62.247997");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=64.256,180.72");
        }
        radioMedium: ApskLayeredScalarRadioMedium {
            parameters:
                @display("p=62.247997,287.14398");
        }
        destination: AdhocHost {
            parameters:
                @display("p=500,179.60625");
        }
        source: AdhocHost {
            @display("p=363.448,180.72");
        }
}

network AnalogModelShowcaseNoiseNetwork
{
    @display("bgb=400,400");
    submodules:

        configurator: Ipv4NetworkConfigurator {
            @display("p=35,40");
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            @display("p=32,113");
        }
        visualizer: IntegratedVisualizer {
            @display("p=37,185");
        }
        host1: AdhocHost {
            @display("p=144,79");
        }
        host2: AdhocHost {
            @display("p=105,154");
        }
        host3: AdhocHost {
            @display("p=350,79");
        }
        host4: AdhocHost {
            @display("p=298,160");
        }
        noiseSource: NoiseSource {
            @display("p=221,132");
        }
}
