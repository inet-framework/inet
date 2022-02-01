//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.wireless.throughput;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


network Throughput
{
    @display("bgb=6,4");
    @statistic[throughput](source=liveThroughput(destinationHost.app[0].packetReceived)/1000000; record=figure; targetFigure=panel.throughput; checkSignals=false);
    @statistic[numRcvdPk](source=count(destinationHost.app[0].packetReceived); record=figure; targetFigure=panel.numRcvdPkCounter; checkSignals=false);
    @figure[panel](type=panel; pos=1.5,0.1);
//    @figure[panel.throughput](type=gauge; pos=0,0; size=100,100; minValue=0; maxValue=40; tickSize=5; label="App level throughput [Mbps]");
    @figure[panel.throughput](type=linearGauge; pos=250,50; size=250,30; minValue=0; maxValue=54; tickSize=6; label="Application level throughput [Mbps]");
    @figure[panel.numRcvdPkCounter](type=counter; pos=50,50; label="Packets received"; decimalPlaces=6);
    submodules:
        sourceHost: WirelessHost {
            @display("p=3,2");
        }
        destinationHost: WirelessHost {
            @display("p=4,2");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=0.5,0.5");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=0.5,1");
        }
}
