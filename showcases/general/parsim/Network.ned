package inet.showcases.general.parsim;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;


channel Ethernet100 extends ned.DatarateChannel
{
    datarate = 100Mbps;
    delay = 100us;
}

network ParSimNetworkBackbone
{
    @display("bgb=812,400");
    submodules:
        host1: StandardHost {
            @display("p=56,44");
        }
        host2: StandardHost {
            @display("p=56,147");
        }
        switch1: EtherSwitch {
            @display("p=130,97");
        }
        host3: WirelessHost {
            @display("p=56,232");
        }
        host4: WirelessHost {
            @display("p=56,335");
        }
        AP1: AccessPoint {
            @display("p=130,285");
        }
        router2: Router {
            @display("p=221.34,271.32");
        }
        host5: WirelessHost {
            @display("p=585,45");
        }
        host6: WirelessHost {
            @display("p=585,154");
        }
        host7: StandardHost {
            @display("p=585,236");
        }
        host8: StandardHost {
            @display("p=591,325");
        }
        AP2: AccessPoint {
            @display("p=515,97");
        }
        switch4: EtherSwitch {
            @display("p=503,280");
        }
        configurator1: Ipv4NetworkConfigurator {
            @display("p=736,47");
        }
        configurator2: Ipv4NetworkConfigurator {
            @display("p=736,147");
        }
        configurator3: Ipv4NetworkConfigurator {
            @display("p=736,247");
        }
        configurator4: Ipv4NetworkConfigurator {
            @display("p=736,347");
        }
        radioMedium2: Ieee80211ScalarRadioMedium {
            @display("p=467,46");
        }
        radioMedium1: Ieee80211ScalarRadioMedium {
            @display("p=193,340");
        }
        router1: Router {
            @display("p=222,112");
        }
        router3: Router {
            @display("p=371,112");
        }
        router4: Router {
            @display("p=370.26,271.32");
        }
    connections:
        host1.ethg++ <--> Ethernet100 <--> switch1.ethg++;
        host2.ethg++ <--> Ethernet100 <--> switch1.ethg++;
        switch4.ethg++ <--> Ethernet100 <--> host7.ethg++;
        switch4.ethg++ <--> Ethernet100 <--> host8.ethg++;
        router1.ethg++ <--> Ethernet100 <--> router3.ethg++;
        router3.ethg++ <--> Ethernet100 <--> router4.ethg++;
        router4.ethg++ <--> Ethernet100 <--> router2.ethg++;
        router2.ethg++ <--> Ethernet100 <--> router1.ethg++;
        router1.ethg++ <--> Ethernet100 <--> router4.ethg++;
        router2.ethg++ <--> Ethernet100 <--> router3.ethg++;
        switch1.ethg++ <--> Ethernet100 <--> router1.ethg++;
        router3.ethg++ <--> Ethernet100 <--> AP2.ethg++;
        AP1.ethg++ <--> Ethernet100 <--> router2.ethg++;
        router4.ethg++ <--> Ethernet100 <--> switch4.ethg++;
}
