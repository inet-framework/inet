
package inet.showcases.wireless.uwbir;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.ieee802154.bitlevel.Ieee802154UwbIrRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network UwbirShowcase
{
    @display("bgb=40,40");
    submodules:
        source: AdhocHost {
            @display("p=24.967382,18.158096;i=misc/smartphone");
        }
        destination: AdhocHost {
            @display("p=28.705814,21.56274;i=misc/smartphone");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=7.8,5.0");
        }
        visualizer: IntegratedVisualizer {
            @display("p=7.8,13.0");
        }
        radioMedium: Ieee802154UwbIrRadioMedium {
            @display("p=7.8,20.5");
        }
}

