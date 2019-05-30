package inet.showcases.emulation.basic;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.AdhocHost;
import inet.node.inet.StandardHost;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


//
// Both sender and receiver are simulated but may be partially external.
//
network NoExtHostsEthernetShowcase
{
    parameters:
        @display("bgb=600,300");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        sender: StandardHost {
            parameters:
                @display("p=300,150");
        }
        receiver: StandardHost {
            parameters:
                @display("p=500,150");
        }
    connections allowunconnected:
        sender.ethg++ <--> Eth100M <--> receiver.ethg++;
}

//
// sender is simulated and receiver is external.
//
network ExtReceiverEthernetShowcase
{
    parameters:
        @display("bgb=600,300");
        @figure[link](type=line; points=320,150,480,150; lineOpacity=0.2);
        @figure[receiver](type=icon; image="device/pc2"; pos=500,150; opacity=0.2);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        sender: StandardHost {
            parameters:
                @display("p=300,150");
        }
    connections allowunconnected:
}

//
// receiver is external
//
network ExtSenderEthernetShowcase
{
    parameters:
        @display("bgb=600,300");
        @figure[link](type=line; points=320,150,480,150; lineOpacity=0.2);
        @figure[receiver](type=icon; image="device/pc2"; pos=300,150; opacity=0.2);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        receiver: StandardHost {
            parameters:
                @display("p=500,150");
        }
    connections allowunconnected:
}


//
// Both sender and receiver are simulated but may be partially external.
//
network NoExtHostsIeee80211Showcase
{
    parameters:
        @display("bgb=600,300");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        sender: AdhocHost {
            parameters:
                @display("p=300,150");
        }
        receiver: AdhocHost {
            parameters:
                @display("p=500,150");
        }
}

//
// sender is simulated and receiver is external.
//
network ExtReceiverIeee80211Showcase
{
    parameters:
        @display("bgb=600,300");
        @figure[link](type=line; points=320,150,480,150; lineOpacity=0.2);
        @figure[receiver](type=icon; image="device/pc2"; pos=500,150; opacity=0.2);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        sender: AdhocHost {
            parameters:
                @display("p=300,150");
        }
    connections allowunconnected:
}

//
// receiver is external
//
network ExtSenderIeee80211Showcase
{
    parameters:
        @display("bgb=600,300");
        @figure[link](type=line; points=320,150,480,150; lineOpacity=0.2);
        @figure[sender](type=icon; image="device/pc2"; pos=300,150; opacity=0.2);
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        receiver: AdhocHost {
            parameters:
                @display("p=500,150");
        }
    connections allowunconnected:
}
