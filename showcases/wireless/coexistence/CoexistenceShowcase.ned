//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.wireless.coexistence;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.common.medium.RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;


network CoexistenceShowcase
{
    @statistic[bitErrorRate](title="Bit error rate"; source=bitErrorRate(packetSentToUpper); record=vector);
    @statistic[packetErrorRate](title="Packet error rate"; source=packetErrorRate(packetSentToUpper); record=vector);
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=5,5");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=5,10");
        }
        radioMedium: RadioMedium {
            @display("p=5,15");
        }
        wifiHost1: AdhocHost {
            @display("p=40.791748,11.9189995;i=device/laptop");
        }
        wifiHost2: AdhocHost {
            @display("p=40.791748,31.4415;i=device/laptop");
        }
        wpanHost1: AdhocHost {
            @display("p=23.426999,21.5775;i=misc/sensor2");
        }
        wpanHost2: AdhocHost {
            @display("p=51.1695,21.5775;i=misc/sensor2");
        }
}
