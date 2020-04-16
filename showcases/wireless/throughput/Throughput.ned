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

package inet.showcases.wireless.throughput;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


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
