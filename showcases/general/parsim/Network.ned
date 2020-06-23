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

network ParSimNetwork
{
    @display("bgb=400,400");
    submodules:
        host1: StandardHost {
            @display("p=50,200");
        }
        host2: StandardHost {
            @display("p=350,200");
        }
    connections:
        //        host1.ethg++ <--> Eth100M <--> host2.ethg++;
        host1.ethg++ <--> Ethernet100 <--> host2.ethg++;
}

network ParSimNetwork2 extends ParSimNetwork
{
    submodules:

        configurator: Ipv4NetworkConfigurator {
            @display("p=35,48");
        }
}

network ParSimNetwork3 extends ParSimNetwork
{
    submodules:

        configurator1: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        configurator2: Ipv4NetworkConfigurator {
            @display("p=350,50");
        }
}

network ParSimNetwork4
{
    @display("bgb=538,400");
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
        host3: StandardHost {
            @display("p=56,232");
        }
        host4: StandardHost {
            @display("p=56,335");
        }
        switch2: EtherSwitch {
            @display("p=130,285");
        }
        router: Router {
            @display("p=190,194");
        }
        host5: StandardHost {
            @display("p=350,38");
        }
        host6: StandardHost {
            @display("p=350,147");
        }
        host7: StandardHost {
            @display("p=350,229");
        }
        host8: StandardHost {
            @display("p=356,318");
        }
        switch3: EtherSwitch {
            @display("p=280,90");
        }
        switch4: EtherSwitch {
            @display("p=268,273");
        }
        configurator1: Ipv4NetworkConfigurator {
            @display("p=477,44");
        }
        configurator2: Ipv4NetworkConfigurator {
            @display("p=477,141");
        }
    connections:
        host1.ethg++ <--> Ethernet100 <--> switch1.ethg++;
        host2.ethg++ <--> Ethernet100 <--> switch1.ethg++;
        host3.ethg++ <--> Ethernet100 <--> switch2.ethg++;
        host4.ethg++ <--> Ethernet100 <--> switch2.ethg++;
        switch1.ethg++ <--> Ethernet100 <--> router.ethg++;
        switch2.ethg++ <--> Ethernet100 <--> router.ethg++;
        router.ethg++ <--> Ethernet100 <--> switch3.ethg++;
        router.ethg++ <--> Ethernet100 <--> switch4.ethg++;
        switch3.ethg++ <--> Ethernet100 <--> host5.ethg++;
        switch3.ethg++ <--> Ethernet100 <--> host6.ethg++;
        switch4.ethg++ <--> Ethernet100 <--> host7.ethg++;
        switch4.ethg++ <--> Ethernet100 <--> host8.ethg++;
}

network ParSimNetwork4FourPartitions extends ParSimNetwork4
{
    submodules:
        configurator3: Ipv4NetworkConfigurator {
            @display("p=477,44");
        }
        configurator4: Ipv4NetworkConfigurator {
            @display("p=477,141");
        }
}

network ParSimNetwork5
{
    @display("bgb=538,400");
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
        router: Router {
            @display("p=190,194");
        }
        host5: WirelessHost {
            @display("p=350,38");
        }
        host6: WirelessHost {
            @display("p=350,147");
        }
        host7: StandardHost {
            @display("p=350,229");
        }
        host8: StandardHost {
            @display("p=356,318");
        }
        AP2: AccessPoint {
            @display("p=280,90");
        }
        switch4: EtherSwitch {
            @display("p=268,273");
        }
        configurator1: Ipv4NetworkConfigurator {
            @display("p=477,44");
        }
        configurator2: Ipv4NetworkConfigurator {
            @display("p=477,144");
        }
        configurator3: Ipv4NetworkConfigurator {
            @display("p=477,244");
        }
        configurator4: Ipv4NetworkConfigurator {
            @display("p=477,344");
        }
        radioMedium1: Ieee80211ScalarRadioMedium {
            @display("p=204,44");
        }
        radioMedium2: Ieee80211ScalarRadioMedium {
            @display("p=193,340");
        }
    connections:
        host1.ethg++ <--> Ethernet100 <--> switch1.ethg++;
        host2.ethg++ <--> Ethernet100 <--> switch1.ethg++;
        switch1.ethg++ <--> Ethernet100 <--> router.ethg++;
        router.ethg++ <--> Ethernet100 <--> AP2.ethg++;
        router.ethg++ <--> Ethernet100 <--> switch4.ethg++;
        switch4.ethg++ <--> Ethernet100 <--> host7.ethg++;
        switch4.ethg++ <--> Ethernet100 <--> host8.ethg++;
        AP1.ethg++ <--> Ethernet100 <--> router.ethg++;
}
