package inet.examples.bgpv4.BgpAndOspfSimple;

import inet.common.misc.ThruputMeteringChannel;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.bgp.BgpRouter;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;
import inet.visualizer.integrated.IntegratedCanvasVisualizer;
import ned.TransmissionChannel;


network BgpNetwork
{
    parameters:
        *.eth[*].bitrate = 100Mbps;
    types:
        channel LINK_100 extends ThruputMeteringChannel
        {
            delay = 50us;
            datarate = 100Mbps;
            displayAsTooltip = true;
            thruputDisplayFormat = "#N";
        }
        channel ethlink extends TransmissionChannel
        {
            delay = 50us;
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
                @display("p=550,100");
            gates:
                pppg[2];
        }
        B: BgpRouter {
            parameters:
                @display("p=700,100");
            gates:
                pppg[2];
        }
        RA1: OspfRouter {
            parameters:
                @display("p=400,100;i=device/router");
            gates:
                pppg[1];
                ethg[1];
        }
        RB1: OspfRouter {
            parameters:
                @display("p=850,100;i=device/router");
            gates:
                pppg[1];
                ethg[1];
        }
        HA11: StandardHost {
            parameters:
                @display("p=250,100;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=1000,100;i=device/pc");
            gates:
                ethg[1];
        }
    connections:
        //LAN A1 et A2
        HA11.ethg[0] <--> ethlink <--> RA1.ethg[0];

        //OSPF A
        RA1.pppg[0] <--> LINK_100 <--> A.pppg[1];

        //BGP
        A.pppg[0] <--> LINK_100 <--> B.pppg[0];

        //LAN B1 et B2
        HB11.ethg[0] <--> ethlink <--> RB1.ethg[0];

        //OSPF B
        RB1.pppg[0] <--> LINK_100 <--> B.pppg[1];
}

