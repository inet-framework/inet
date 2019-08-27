package inet.examples.scalabilityTest;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.apskradio.packetlevel.ApskScalarRadioMedium;


network TxopShowcase
{
    int numHosts;
    @display("bgb=675,400");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        radioMedium: ApskScalarRadioMedium {
            @display("p=50,100");
        }
        host1: AdhocHost {
            @display("p=80,110;i=misc/smartphone");
        }
        host2: AdhocHost {
            @display("p=80,110.000001;i=misc/smartphone");
        }
        extrahost[numHosts]: AdhocHost;
}
