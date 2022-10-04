//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.wireless.multiradio;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;


network MultiRadioShowcase
{
    parameters:
        @display("bgb=657,300");
    submodules:
        visualizer: IntegratedMultiCanvasVisualizer {
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
        host1: WirelessHost {
            parameters:
                @display("p=227,203");
        }
        host2: WirelessHost {
            parameters:
                @display("p=531,203");
        }
        accessPoint: AccessPoint {
            parameters:
                @display("p=384,100");
        }
}
