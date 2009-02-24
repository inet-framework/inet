
package inet.examples.ospfv2.backbone;

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.OSPFRouter;
import inet.nodes.inet.StandardHost;
import inet.world.ChannelInstaller;
import ned.DatarateChannel;


network Backbone
{
    parameters:
        @display("p=10,10;b=736,568");
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
        R1: OSPFRouter {
            parameters:
                @display("p=372,340");
            gates:
                ethg[3];
        }
        N1: EtherHub {
            parameters:
                @display("p=368,268");
            gates:
                ethg[2];
        }
        N2: EtherHub {
            parameters:
                @display("p=452,384");
            gates:
                ethg[3];
        }
        R2: OSPFRouter {
            parameters:
                @display("p=368,188");
            gates:
                ethg[2];
        }
        R4: OSPFRouter {
            parameters:
                @display("p=216,432");
            gates:
                ethg[2];
        }
        R3: OSPFRouter {
            parameters:
                @display("p=532,428");
            gates:
                ethg[2];
        }
        N4: EtherHub {
            parameters:
                @display("p=368,100");
            gates:
                ethg[4];
        }
        N5: EtherHub {
            parameters:
                @display("p=612,476");
            gates:
                ethg[3];
        }
        R5: OSPFRouter {
            parameters:
                @display("p=488,284");
            gates:
                ethg[2];
        }
        R7: OSPFRouter {
            parameters:
                @display("p=256,284");
            gates:
                ethg[2];
        }
        R6: OSPFRouter {
            parameters:
                @display("p=372,476");
            gates:
                ethg[2];
        }
        N6: EtherHub {
            parameters:
                @display("p=136,476");
            gates:
                ethg[3];
        }
        N3: EtherHub {
            parameters:
                @display("p=292,384");
            gates:
                ethg[2];
        }
        H1: StandardHost {
            parameters:
                @display("p=372,420;i=device/laptop");
            gates:
                ethg[1];
        }
        H2: StandardHost {
            parameters:
                @display("p=368,40;i=device/laptop");
            gates:
                ethg[1];
        }
    connections:
        R1.ethg[0] <--> C <--> N1.ethg[1];
        R1.ethg[1] <--> C <--> N2.ethg[2];
        N1.ethg[0] <--> C <--> R2.ethg[1];
        R2.ethg[0] <--> C <--> N4.ethg[2];
        N4.ethg[1] <--> C <--> R5.ethg[1];
        R5.ethg[0] <--> C <--> N5.ethg[2];
        N2.ethg[0] <--> C <--> R3.ethg[1];
        R3.ethg[0] <--> C <--> N5.ethg[1];
        N5.ethg[0] <--> C <--> R6.ethg[0];
        N6.ethg[2] <--> C <--> R6.ethg[1];
        R1.ethg[2] <--> C <--> N3.ethg[0];
        N3.ethg[1] <--> C <--> R4.ethg[0];
        R4.ethg[1] <--> C <--> N6.ethg[1];
        N4.ethg[3] <--> C <--> R7.ethg[0];
        R7.ethg[1] <--> C <--> N6.ethg[0];
        N4.ethg[0] <--> C <--> H2.ethg[0];
        N2.ethg[1] <--> C <--> H1.ethg[0];
}

