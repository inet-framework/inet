package inet.test.moduletest.lib;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.idealradio.UnitDiskRadioMedium;


network NetIdealRadios
{
    parameters:
        int numHosts;
        @display("bgb=600,400");
    submodules:
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=250,58");
        }
        radioMedium: UnitDiskRadioMedium {
            parameters:
                @display("p=60,50");
        }
        configurator: Ipv4NetworkConfigurator {
            config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.0.0'/></config>");
            @display("p=140,50");
        }
}

