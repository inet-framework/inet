//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.canvas.styling;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network StylingShowcase
{
    @display("bgb=600,250");
    submodules:
        host1: AdhocHost {
            @display("p=205,150");
        }
        host2: AdhocHost {
            @display("p=465,150");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=50,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,100");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=50,150");
        }
}

network AnnotationShowcase
{
    @display("bgb=600,250");
    submodules:
        host1: WirelessHost {
            @display("p=205,150");
        }
        host2: WirelessHost {
            @display("p=465,150");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=50,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,100");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=50,150");
        }
        accessPoint: AccessPoint {
            @display("p=329,67");
        }
}

