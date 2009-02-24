


package inet.examples.ospfv2.fulltest;

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.OSPFRouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import ned.DatarateChannel;


network OSPF_TestNetwork
{
    parameters:
        @display("p=10,10;b=704,560");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=#N";
                @display("p=98,50");
        }
        Area1: OSPF_Area1 {
            parameters:
                @display("p=120,172;b=176,168");
            gates:
                ethg[3];
        }
        Area2: OSPF_Area2 {
            parameters:
                @display("p=448,468;b=176,168");
            gates:
                ethg[4];
        }
        Area3: OSPF_Area3 {
            parameters:
                @display("p=128,468;b=176,168");
            gates:
                ethg[1];
        }
        RT3: OSPFRouter {
            parameters:
                @display("p=276,208");
            gates:
                ethg[3];
        }
        RT4: OSPFRouter {
            parameters:
                @display("p=276,144");
            gates:
                ethg[2];
        }
        RT5: OSPFRouter {
            parameters:
                @display("p=496,144");
            gates:
                ethg[6];
        }
        RT6: OSPFRouter {
            parameters:
                @display("p=424,208");
            gates:
                ethg[3];
        }
        RT7: OSPFRouter {
            parameters:
                @display("p=496,328");
            gates:
                ethg[4];
        }
        RT10: OSPFRouter {
            parameters:
                @display("p=424,328");
            gates:
                ethg[3];
        }
        RT11: OSPFRouter {
            parameters:
                @display("p=284,468");
            gates:
                ethg[2];
        }
        N12: EtherHub {
            parameters:
                @display("p=608,296");
            gates:
                ethg[3];
        }
        N13: EtherHub {
            parameters:
                @display("p=500,92");
            gates:
                ethg[2];
        }
        N14: EtherHub {
            parameters:
                @display("p=608,92");
            gates:
                ethg[2];
        }
        N15: EtherHub {
            parameters:
                @display("p=608,356");
            gates:
                ethg[2];
        }
        H18: StandardHost {
            parameters:
                @display("p=496,36;i=device/pc_s");
            gates:
                ethg[1];
        }
        H19: StandardHost {
            parameters:
                @display("p=612,36;i=device/pc_s");
            gates:
                ethg[1];
        }
        H20: StandardHost {
            parameters:
                @display("p=688,296;i=device/pc_s");
            gates:
                ethg[1];
        }
        H21: StandardHost {
            parameters:
                @display("p=688,356;i=device/pc_s");
            gates:
                ethg[1];
        }
    connections:
        RT11.ethg[0] <--> C <--> Area3.ethg[0];
        RT11.ethg[1] <--> C <--> Area2.ethg[0];
        RT3.ethg[0] <--> C { @display("m=m,41,50,88,40"); } <--> Area1.ethg[1];
        RT3.ethg[1] <--> C { @display("m=m,77,42,92,96"); } <--> Area1.ethg[2];
        N13.ethg[1] <--> C <--> RT5.ethg[0];
        N12.ethg[0] <--> C <--> RT5.ethg[4];
        RT5.ethg[5] <--> C <--> N14.ethg[1];
        RT3.ethg[2] <--> C <--> RT6.ethg[0];
        RT10.ethg[0] <--> C <--> RT6.ethg[1];
        RT7.ethg[3] <--> C <--> N12.ethg[1];
        RT7.ethg[2] <--> C <--> N15.ethg[0];
        RT5.ethg[3] <--> C <--> RT7.ethg[0];
        RT5.ethg[2] <--> C <--> RT6.ethg[2];
        RT10.ethg[1] <--> C { @display("m=m,50,50,0,30"); } <--> Area2.ethg[1];
        RT10.ethg[2] <--> C { @display("m=m,50,50,64,12"); } <--> Area2.ethg[2];
        RT7.ethg[1] <--> C <--> Area2.ethg[3];
        RT4.ethg[1] <--> C <--> RT5.ethg[1];
        RT4.ethg[0] <--> C <--> Area1.ethg[0];
        N14.ethg[0] <--> C <--> H19.ethg[0];
        H20.ethg[0] <--> C <--> N12.ethg[2];
        H21.ethg[0] <--> C <--> N15.ethg[1];
        H18.ethg[0] <--> C <--> N13.ethg[0];
}

