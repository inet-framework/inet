//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.visualizer.canvas.instrumentfigures;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.common.IntegratedVisualizer;

network InstrumentShowcase
{
    @display("bgb=900,500");

    @statistic[throughput](source=liveThroughput(client.app[0].packetReceived)/1000000; record=figure; targetFigure=throughputGauge; checkSignals=false);
    @figure[throughputGauge](type=gauge; pos=190,30; size=100,100; minValue=0; maxValue=25; tickSize=5; label="App level throughput [Mbps]");

    @statistic[bitrate1](source=server.wlan[0].mac.dcf.rateControl.datarateChanged/1000000; record=figure; targetFigure=bitrateLinearGauge);
    @figure[bitrateLinearGauge](type=linearGauge; pos=320,65; size=250,30; minValue=0; maxValue=54; tickSize=6; label="WiFi bit rate [Mbps]");

    @statistic[progress](source=100 * sum(packetBytes(client.app[0].packetReceived)) / 1048576; record=figure; targetFigure=progressMeter; checkSignals=false);
    @figure[progressMeter](type=progressMeter; pos=200,180; size=420,20; text="%.4g%%"; label="Download progress");

    @statistic[numRcvdPk](source=count(client.app[0].packetReceived); record=figure; targetFigure=numRcvdPkCounter; checkSignals=false);
    @figure[numRcvdPkCounter](type=counter; pos=760,260; label="Packets received"; decimalPlaces=4);

    @statistic[per1](source=packetErrorRate(client.wlan[0].radio.packetSentToUpper); record=figure; targetFigure=perThermometer);
    @figure[perThermometer](type=thermometer; pos=600,30; size=30,100; minValue=0; maxValue=1; tickSize=0.2; label="Packet error rate");

    @statistic[ctn](source=server.wlan[0].mac.dcf.channelAccess.contention.stateChanged; record=figure; targetFigure=ctnIndexedImage); // note: indexedImage takes the value modulo the number of images
    @figure[ctnIndexedImage](type=indexedImage; pos=800,340; size=32,32; images=showcases/idle,showcases/listen,showcases/clock; label="Contention state"; labelOffset=0,25; interpolation=best);

    @statistic[per2](source=packetErrorRate(client.wlan[0].radio.packetSentToUpper); record=figure; targetFigure=perPlot);
    @figure[perPlot](type=plot; pos=700,20; size=150,60; timeWindow=3; maxY=1; yTickSize=1; label="Packet error rate");

    @statistic[bitrate2](source=server.wlan[0].mac.dcf.rateControl.datarateChanged/1000000; record=figure; targetFigure=bitratePlot);
    @figure[bitratePlot](type=plot; pos=700,140; size=150,60; timeWindow=3; maxY=55; yTickSize=54; label="WiFi bit rate [Mbps]");

    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=62,41");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=61,110");
        }
        visualizer: IntegratedVisualizer {
            @display("p=62,170");
        }
        client: AdhocHost {
            @display("p=300,350");
        }
        server: AdhocHost {
            @display("p=250,350");
        }
}
