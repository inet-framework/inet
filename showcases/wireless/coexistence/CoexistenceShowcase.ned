//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.showcases.wireless.coexistence;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.wireless.Probe;
import inet.physicallayer.common.packetlevel.RadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;

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
