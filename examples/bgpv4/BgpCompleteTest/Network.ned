package inet.examples.bgpv4.BgpCompleteTest;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.bgp.BgpRouter;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;

network BgpNetwork
{
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            parameters:
                delay = 50us;
                datarate = 100Mbps;
                displayAsTooltip = true;
                thruputDisplayFormat = "#N";
        }
    submodules:
        visualizer: IntegratedCanvasVisualizer {
            parameters:
                @display("p=100,100;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,200;is=s");
                config = xmldoc("IPv4Config.xml");
                addStaticRoutes = false;
                addDefaultRoutes = false;
                addSubnetRoutes = false;
        }
        A1: BgpRouter {
            parameters:
                @display("p=600,80");
            gates:
                pppg[2];
        }
        A2: BgpRouter {
            parameters:
                @display("p=450,240");
            gates:
                pppg[2];
        }
        B1: BgpRouter {
            parameters:
                @display("p=450,400");
            gates:
                pppg[2];
                ethg[1];
        }
        C1: BgpRouter {
            parameters:
                @display("p=700,160");
            gates:
                pppg[2];
        }
        C2: BgpRouter {
            parameters:
                @display("p=625,320");
            gates:
                pppg[2];
        }
        C3: BgpRouter {
            parameters:
                @display("p=900,160");
            gates:
                pppg[2];
        }
        C4: BgpRouter {
            parameters:
                @display("p=900,320");
            gates:
                pppg[2];
        }
        D1: BgpRouter {
            parameters:
                @display("p=1000,80");
            gates:
                pppg[1];
                ethg[1];
        }
        E1: BgpRouter {
            parameters:
                @display("p=1000,400");
            gates:
                pppg[1];
                ethg[1];
        }
        RA3: OspfRouter {
            parameters:
                @display("p=450,80;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RC5: OspfRouter {
            parameters:
                @display("p=800,240;i=device/router");
            gates:
                pppg[4];
        }
        PA3: EtherSwitch {
            parameters:
                @display("p=250,80;i=device/switch");
            gates:
                ethg[3];
        }
        PB1: EtherSwitch {
            parameters:
                @display("p=250,400;i=device/switch");
            gates:
                ethg[3];
        }
        PD1: EtherSwitch {
            parameters:
                @display("p=1150,80;i=device/switch");
            gates:
                ethg[3];
        }
        PE1: EtherSwitch {
            parameters:
                @display("p=1150,400;i=device/switch");
            gates:
                ethg[3];
        }
        HA11: StandardHost {
            parameters:
                @display("p=175,160;i=device/pc");
            gates:
                ethg[1];
        }
        HA12: StandardHost {
            parameters:
                @display("p=325,160;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=325,480;i=device/pc");
            gates:
                ethg[1];
        }
        HB12: StandardHost {
            parameters:
                @display("p=175,480;i=device/pc");
            gates:
                ethg[1];
        }
        HD11: StandardHost {
            parameters:
                @display("p=1075,160;i=device/pc");
            gates:
                ethg[1];
        }
        HD12: StandardHost {
            parameters:
                @display("p=1225,160;i=device/pc");
            gates:
                ethg[1];
        }
        HE11: StandardHost {
            parameters:
                @display("p=1075,480;i=device/pc");
            gates:
                ethg[1];
        }
        HE12: StandardHost {
            parameters:
                @display("p=1225,480;i=device/pc");
            gates:
                ethg[1];
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

