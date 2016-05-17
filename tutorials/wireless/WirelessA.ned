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

package inet.tutorials.wireless;

import inet.common.figures.DelegateSignalConfigurator;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.INetworkNode;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;


network WirelessA
{
    parameters:
        string hostType = default("WirelessHost");
        string mediumType = default("IdealRadioMedium");

        @display("bgb=650,500;bgg=100,1,grey95");

        @figure[sentPk](type=progressMeter; pos=80,50; size=100,15; anchor=c; min=0; max=2500; cornerRadius=3; text="%g (%g%%)"; label=# packets sent);
        @statistic[sentPk](source=hostA_sentPk; record=figure(count); meterFigure=sentPk);
        @signal[hostA_sentPk];
        @delegatesignal[sentPk](source=hostA.udpApp[0].sentPk; target=hostA_sentPk);

        @figure[rcvdPk](type=progressMeter; pos=430,50; size=100,15; anchor=c; min=0; max=2500; cornerRadius=3; text="%g (%g%%)"; label=# packets received);
        @statistic[rcvdPk](source=hostB_rcvdPk; record=figure(count); meterFigure=rcvdPk);
        @signal[hostB_rcvdPk];
        @delegatesignal[rcvdPk](source=hostB.udpApp[0].rcvdPk; target=hostB_rcvdPk);

        @figure[ratio](type=textMeter; pos=200,57; anchor=nw; textFormat=arrival ratio: %.2f%%);
        @statistic[ratio](source=100*count(hostB_rcvdPk)/count(hostA_sentPk); record=figure(last); meterFigure=ratio);

    submodules:
        visualizer: IntegratedCanvasVisualizer {
            @display("p=580,200");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=580,275");
        }
        radioMedium: <mediumType> like IRadioMedium {
            @display("p=580,350");
        }
        figureHelper: DelegateSignalConfigurator {
            @display("p=580,425");
        }
        hostA: <hostType> like INetworkNode {
            @display("p=50,325");
        }
        hostB: <hostType> like INetworkNode {
            @display("p=450,325");
        }
}

