package inet.examples.bgpv4.BGPCompleteTest;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.bgp.BGPRouter;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OSPFRouter;


network BGPTest
{
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            parameters:
                delay = 0;
                datarate = 100Mbps;
                thruputDisplayFormat = "#N";
        }
    submodules:
        A1: BGPRouter {
            parameters:
                @display("p=178,45");
            gates:
                pppg[2];
        }
        A2: BGPRouter {
            parameters:
                @display("p=168,93");
            gates:
                pppg[2];
        }
        B1: BGPRouter {
            parameters:
                @display("p=194,253");
            gates:
                pppg[2];
                ethg[1];
        }
        C1: BGPRouter {
            parameters:
                @display("p=322,45");
            gates:
                pppg[2];
        }
        C2: BGPRouter {
            parameters:
                @display("p=322,141");
            gates:
                pppg[2];
        }
        C3: BGPRouter {
            parameters:
                @display("p=450,45");
            gates:
                pppg[2];
        }
        C4: BGPRouter {
            parameters:
                @display("p=450,149");
            gates:
                pppg[2];
        }
        D1: BGPRouter {
            parameters:
                @display("p=562,141");
            gates:
                pppg[1];
                ethg[1];
        }
        E1: BGPRouter {
            parameters:
                @display("p=514,269");
            gates:
                pppg[1];
                ethg[1];
        }
        RA3: OSPFRouter {
            parameters:
                @display("p=102,43;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RC5: OSPFRouter {
            parameters:
                @display("p=390,97;i=device/router");
            gates:
                pppg[4];
        }
        PA3: EtherSwitch {
            parameters:
                @display("p=52,86;i=device/switch");
            gates:
                ethg[3];
        }
        PB1: EtherSwitch {
            parameters:
                @display("p=132,302;i=device/switch");
            gates:
                ethg[3];
        }
        PD1: EtherSwitch {
            parameters:
                @display("p=612,78;i=device/switch");
            gates:
                ethg[3];
        }
        PE1: EtherSwitch {
            parameters:
                @display("p=452,318;i=device/switch");
            gates:
                ethg[3];
        }
        HA11: StandardHost {
            parameters:
                @display("p=58,146;i=device/pc");
            gates:
                ethg[1];
        }
        HA12: StandardHost {
            parameters:
                @display("p=114,146;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=246,306;i=device/pc");
            gates:
                ethg[1];
        }
        HB12: StandardHost {
            parameters:
                @display("p=206,338;i=device/pc");
            gates:
                ethg[1];
        }
        HD11: StandardHost {
            parameters:
                @display("p=582,194;i=device/pc");
            gates:
                ethg[1];
        }
        HD12: StandardHost {
            parameters:
                @display("p=630,194;i=device/pc");
            gates:
                ethg[1];
        }
        HE11: StandardHost {
            parameters:
                @display("p=518,364;i=device/pc");
            gates:
                ethg[1];
        }
        HE12: StandardHost {
            parameters:
                @display("p=550,316;i=device/pc");
            gates:
                ethg[1];
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=58,351");
            config = xmldoc("IPv4Config.xml");
            //assignAddresses = false;
            //assignDisjunctSubnetAddresses = false;
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
    connections:
        //LAN RA3
        HA11.ethg[0] <--> LINK_100 <--> PA3.ethg[0];
        HA12.ethg[0] <--> LINK_100 <--> PA3.ethg[1];
        PA3.ethg[2] <--> LINK_100 <--> RA3.ethg[0];

        //LAN B1
        HB11.ethg[0] <--> LINK_100 <--> PB1.ethg[0];
        HB12.ethg[0] <--> LINK_100 <--> PB1.ethg[1];
        PB1.ethg[2] <--> LINK_100 <--> B1.ethg[0];

        //LAN D1
        HD11.ethg[0] <--> LINK_100 <--> PD1.ethg[0];
        HD12.ethg[0] <--> LINK_100 <--> PD1.ethg[1];
        PD1.ethg[2] <--> LINK_100 <--> D1.ethg[0];

        //LAN E1
        HE11.ethg[0] <--> LINK_100 <--> PE1.ethg[0];
        HE12.ethg[0] <--> LINK_100 <--> PE1.ethg[1];
        PE1.ethg[2] <--> LINK_100 <--> E1.ethg[0];

        //OSPF A
        RA3.pppg[0] <--> LINK_100 <--> A1.pppg[1];
        RA3.pppg[1] <--> LINK_100 <--> A2.pppg[1];

        //OSPF C
        RC5.pppg[0] <--> LINK_100 <--> C1.pppg[1];
        RC5.pppg[1] <--> LINK_100 <--> C2.pppg[1];
        RC5.pppg[2] <--> LINK_100 <--> C3.pppg[1];
        RC5.pppg[3] <--> LINK_100 <--> C4.pppg[1];

        //BGP
        A1.pppg[0] <--> LINK_100 <--> C1.pppg[0];
        A2.pppg[0] <--> LINK_100 <--> B1.pppg[0];
        B1.pppg[1] <--> LINK_100 <--> C2.pppg[0];
        C3.pppg[0] <--> LINK_100 <--> D1.pppg[0];
        C4.pppg[0] <--> LINK_100 <--> E1.pppg[0];
}

