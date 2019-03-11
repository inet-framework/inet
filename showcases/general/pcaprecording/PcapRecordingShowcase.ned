package inet.showcases.general.pcaprecording;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;
import inet.visualizer.canvas.integrated.IntegratedMultiCanvasVisualizer;


network PcapRecordingShowcase
{
    @display("bgb=1214,400");
    @statistic[progress](source=100 * sum(packetBytes(ethHost2.app[0].packetReceived)) / 1048576; record=figure; targetFigure=progressMeter; checkSignals=false);
    @figure[progressMeter](type=progressMeter; pos=900,300; size=250,20; text="%.4g%%"; label="eth2Host: download progress");
    @statistic[numRcvdPk](source=count(host2.app[0].packetReceived); record=figure; targetFigure=numRcvdPkCounter; checkSignals=false);
    @figure[numRcvdPkCounter](type=counter; pos=400,40; label="host2: packets received"; decimalPlaces=4);
    @statistic[rtt](source=ethHost1.app[1].rtt; record=figure; targetFigure=rttThermometer);
    @figure[rttThermometer](type=thermometer; pos=400,250; size=30,100; minValue=0; maxValue=0.002; label="ethHost1: ping rtt");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=87,57");
        }
        visualizer: IntegratedMultiCanvasVisualizer {
            @display("p=87,130");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=86,192");
        }
        host1: AdhocHost {
            @display("p=201,243");
        }
        host2: AdhocHost {
            @display("p=289,115");
        }
        ethHost1: StandardHost {
            @display("p=405,176");
        }
        ethHost2: StandardHost {
            @display("p=1126,176");
        }
        router1: Router {
            @display("p=674,176");
        }
        router2: Router {
            @display("p=891,176");
        }
    connections:
        ethHost1.ethg++ <--> Eth100M <--> router1.ethg++;
        router2.ethg++ <--> Eth100M <--> ethHost2.ethg++;
        router1.pppg++ <--> Eth100M <--> router2.pppg++;
}
