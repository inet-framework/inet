package inet.examples.bgpv4.BgpAndOspf;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.bgp.BgpRouter;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

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
        A: BgpRouter {
            parameters:
                @display("p=700,230");
            gates:
                pppg[3];
        }
        B: BgpRouter {
            parameters:
                @display("p=850,230");
            gates:
                pppg[3];
        }
        RA1: OspfRouter {
            parameters:
                @display("p=550,130;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RA2: OspfRouter {
            parameters:
                @display("p=550,330;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RB1: OspfRouter {
            parameters:
                @display("p=1000,130;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RB2: OspfRouter {
            parameters:
                @display("p=1000,330;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        PA1: EthernetSwitch {
            parameters:
                @display("p=400,130;i=device/switch");
            gates:
                ethg[3];
        }
        PA2: EthernetSwitch {
            parameters:
                @display("p=400,330;i=device/switch");
            gates:
                ethg[3];
        }
        PB1: EthernetSwitch {
            parameters:
                @display("p=1150,130;i=device/switch");
            gates:
                ethg[3];
        }
        PB2: EthernetSwitch {
            parameters:
                @display("p=1150,330;i=device/switch");
            gates:
                ethg[3];
        }
        HA11: StandardHost {
            parameters:
                @display("p=250,80;i=device/pc");
            gates:
                ethg[1];
        }
        HA12: StandardHost {
            parameters:
                @display("p=250,180;i=device/pc");
            gates:
                ethg[1];
        }
        HA21: StandardHost {
            parameters:
                @display("p=250,280;i=device/pc");
            gates:
                ethg[1];
        }
        HA22: StandardHost {
            parameters:
                @display("p=250,380;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=1300,80;i=device/pc");
            gates:
                ethg[1];
        }
        HB12: StandardHost {
            parameters:
                @display("p=1300,180;i=device/pc");
            gates:
                ethg[1];
        }
        HB21: StandardHost {
            parameters:
                @display("p=1300,280;i=device/pc");
            gates:
                ethg[1];
        }
        HB22: StandardHost {
            parameters:
                @display("p=1300,380;i=device/pc");
            gates:
                ethg[1];
        }
    connections:
        //LAN A1 et A2
        HA11.ethg[0] <--> LINK_100 <--> PA1.ethg[0];
        HA12.ethg[0] <--> LINK_100 <--> PA1.ethg[1];
        HA21.ethg[0] <--> LINK_100 <--> PA2.ethg[0];
        HA22.ethg[0] <--> LINK_100 <--> PA2.ethg[1];
        PA1.ethg[2] <--> LINK_100 <--> RA1.ethg[0];
        PA2.ethg[2] <--> LINK_100 <--> RA2.ethg[0];

        //OSPF A
        RA1.pppg[1] <--> LINK_100 <--> RA2.pppg[1];
        RA1.pppg[0] <--> LINK_100 <--> A.pppg[1];
        RA2.pppg[0] <--> LINK_100 <--> A.pppg[2];

        //BGP
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];

        //LAN B1 et B2
        HB11.ethg[0] <--> LINK_100 <--> PB1.ethg[0];
        HB12.ethg[0] <--> LINK_100 <--> PB1.ethg[1];
        HB21.ethg[0] <--> LINK_100 <--> PB2.ethg[0];
        HB22.ethg[0] <--> LINK_100 <--> PB2.ethg[1];
        PB1.ethg[2] <--> LINK_100 <--> RB1.ethg[0];
        PB2.ethg[2] <--> LINK_100 <--> RB2.ethg[0];

        //OSPF B
        RB1.pppg[1] <--> LINK_100 <--> RB2.pppg[1];
        RB1.pppg[0] <--> LINK_100 <--> B.pppg[1];
        RB2.pppg[0] <--> LINK_100 <--> B.pppg[2];
}


