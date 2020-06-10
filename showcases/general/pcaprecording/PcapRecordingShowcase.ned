package inet.showcases.general.pcaprecording;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedMultiVisualizer;
import ned.DatarateChannel;


network PcapRecordingShowcase
{
    @display("bgb=1214,400");
    types:
        channel Ppp100M extends DatarateChannel
        {
            double length @unit(m) = default(10m);
            delay = replaceUnit(length / 2e8, "s");
            datarate = 100Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=87,57");
        }
        visualizer: IntegratedMultiVisualizer {
            @display("p=87,130");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
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
        router1.pppg++ <--> Ppp100M <--> router2.pppg++;
}

