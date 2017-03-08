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

package inet.samples.visualizer.Indicator;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.node.inet.WirelessHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network Indicator
{
    @display("bgb=831,400");
    @figure[text](type=indicatorText; pos=20,50; anchor=w);
    @statistic[text](source=signalSourceOutput; record=figure; targetFigure=text);

    @figure[label](type=indicatorLabel; pos=20,80; anchor=w);
    @statistic[label](source=signalSourceOutput; record=figure; targetFigure=label);

    @figure[gauge](type=gauge; pos=150,50; size=150,150);
    @statistic[gauge](source=signalSourceOutput; record=figure; targetFigure=gauge);

    @figure[lineargauge](type=linearGauge; pos=340,90; size=300,50);
    @statistic[lineargauge](source=signalSourceOutput; record=figure; targetFigure=lineargauge);

    @figure[progress](type=progressMeter; pos=220,230; size=300,20);
    @statistic[progress](source=100 * sum(packetBytes(server.udp.sentPk)) / 1038000; record=figure; targetFigure=progress);

    @figure[counter](type=counter; pos=550,227);
    @statistic[counter](source=count(server.udp.sentPk); record=figure; targetFigure=counter);

    @figure[thermometer](type=thermometer; pos=700,50; size=50,300);
    @statistic[thermometer](source=server.wlan[0].radio.packetErrorRate; record=figure; targetFigure=thermometer);

//    @figure[image](type=indexedImage; pos=240,280; size=16,16; images=status/off,status/green,status/red);
//    @statistic[image](source=xx.wla.ad.signalSourceOutput; record=figure; targetFigure=image); // note: indexedImage takes the value modulo the number of images
    submodules:
        client: AdhocHost {
            @display("p=320,317");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=62,41");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=61,110");
        }
        visualizer: IntegratedVisualizer {
            @display("p=62,170");
        }
        server: WirelessHost {
            @display("p=213,320");
        }
}
