package inet.examples.bgpv4.BGP2RoutersInAS;

import inet.base.UnimplementedModule;
import inet.nodes.bgp.BGPRouter;
import inet.nodes.ethernet.EtherSwitch;
import inet.nodes.inet.StandardHost;
import inet.nodes.ospfv2.OSPFRouter;
import inet.util.ThruputMeteringChannel;


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
                @display("p=274,93");
            gates:
                pppg[2];
        }
        A2: BGPRouter {
            parameters:
                @display("p=184,221");
            gates:
                pppg[2];
        }
        B: BGPRouter {
            parameters:
                @display("p=386,93");
            gates:
                pppg[2];
        }
        C: BGPRouter {
            parameters:
                @display("p=298,221");
            gates:
                pppg[3];
        }
        RA1: OSPFRouter {
            parameters:
                @display("p=198,41;i=device/router");
            gates:
                pppg[3];
        }
        RA2: OSPFRouter {
            parameters:
                @display("p=150,139;i=device/router");
            gates:
                pppg[3];
        }
        RA3: OSPFRouter {
            parameters:
                @display("p=102,43;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RB1: OSPFRouter {
            parameters:
                @display("p=462,145;i=device/router");
            gates:
                pppg[1];
                ethg[1];
        }
        RC1: OSPFRouter {
            parameters:
                @display("p=326,313;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        RC2: OSPFRouter {
            parameters:
                @display("p=398,227;i=device/router");
            gates:
                pppg[2];
                ethg[1];
        }
        PA3: EtherSwitch {
            parameters:
                @display("p=52,102;i=device/switch");
            gates:
                ethg[3];
        }
        PB1: EtherSwitch {
            parameters:
                @display("p=460,62;i=device/switch");
            gates:
                ethg[3];
        }
        PC1: EtherSwitch {
            parameters:
                @display("p=420,318;i=device/switch");
            gates:
                ethg[3];
        }
        PC2: EtherSwitch {
            parameters:
                @display("p=492,230;i=device/switch");
            gates:
                ethg[3];
        }
        HA31: StandardHost {
            parameters:
                @display("p=42,178;i=device/pc");
            gates:
                ethg[1];
        }
        HA32: StandardHost {
            parameters:
                @display("p=98,178;i=device/pc");
            gates:
                ethg[1];
        }
        HB11: StandardHost {
            parameters:
                @display("p=542,50;i=device/pc");
            gates:
                ethg[1];
        }
        HB12: StandardHost {
            parameters:
                @display("p=542,114;i=device/pc");
            gates:
                ethg[1];
        }
        HC11: StandardHost {
            parameters:
                @display("p=390,394;i=device/pc");
            gates:
                ethg[1];
        }
        HC12: StandardHost {
            parameters:
                @display("p=454,394;i=device/pc");
            gates:
                ethg[1];
        }
        HC21: StandardHost {
            parameters:
                @display("p=478,308;i=device/pc");
            gates:
                ethg[1];
        }
        HC22: StandardHost {
            parameters:
                @display("p=526,308;i=device/pc");
            gates:
                ethg[1];
        }
        configurator: UnimplementedModule {
            @display("p=71,374");
        }
    connections:
        //LAN RA3
        HA31.ethg[0] <--> LINK_100 <--> PA3.ethg[0];
        HA32.ethg[0] <--> LINK_100 <--> PA3.ethg[1];
        PA3.ethg[2] <--> LINK_100 <--> RA3.ethg[0];

        //LAN RB1
        HB11.ethg[0] <--> LINK_100 <--> PB1.ethg[0];
        HB12.ethg[0] <--> LINK_100 <--> PB1.ethg[1];
        PB1.ethg[2] <--> LINK_100 <--> RB1.ethg[0];

        //LAN C1 et C2
        HC11.ethg[0] <--> LINK_100 <--> PC1.ethg[0];
        HC12.ethg[0] <--> LINK_100 <--> PC1.ethg[1];
        HC21.ethg[0] <--> LINK_100 <--> PC2.ethg[0];
        HC22.ethg[0] <--> LINK_100 <--> PC2.ethg[1];
        PC1.ethg[2] <--> LINK_100 <--> RC1.ethg[0];
        PC2.ethg[2] <--> LINK_100 <--> RC2.ethg[0];

        //OSPF A
        RA1.pppg[2] <--> LINK_100 <--> RA3.pppg[0];
        RA2.pppg[2] <--> LINK_100 <--> RA3.pppg[1];
        RA1.pppg[1] <--> LINK_100 <--> RA2.pppg[1];
        RA1.pppg[0] <--> LINK_100 <--> A1.pppg[1];
        RA2.pppg[0] <--> LINK_100 <--> A2.pppg[1];

        //OSPF B
        RB1.pppg[0] <--> LINK_100 <--> B.pppg[1];

        //OSPF C
        RC1.pppg[1] <--> LINK_100 <--> RC2.pppg[1];
        RC1.pppg[0] <--> LINK_100 <--> C.pppg[1];
        RC2.pppg[0] <--> LINK_100 <--> C.pppg[2];

        //BGP
        A1.pppg[0] <--> LINK_100 <--> B.pppg[0];
        A2.pppg[0] <--> LINK_100 <--> C.pppg[0];
}

