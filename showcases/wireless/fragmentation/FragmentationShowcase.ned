
package inet.showcases.wireless.fragmentation;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network FragmentationShowcase
{
    @display("bgb=400,400");
    submodules:
        wifiHost1: AdhocHost {
            @display("p=215,99;i=misc/smartphone");
        }
        wifiHost2: AdhocHost {
            @display("p=315,283;i=misc/smartphone");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=78,50");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=78,130");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=78,205");
        }
}

