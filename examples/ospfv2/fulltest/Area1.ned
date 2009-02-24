


package inet.examples.ospfv2.fulltest;

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.OSPFRouter;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


module OSPF_Area1
{
    parameters:
        @display("p=10,42;b=432,504");
    gates:
        inout ethg[];
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        RT1: OSPFRouter {
            parameters:
                @display("p=232,136");
            gates:
                ethg[2];
        }
        RT2: OSPFRouter {
            parameters:
                @display("p=236,348");
            gates:
                ethg[2];
        }
        N1: EtherHub {
            parameters:
                @display("p=140,136");
            gates:
                ethg[3];
        }
        N2: EtherHub {
            parameters:
                @display("p=140,348");
            gates:
                ethg[3];
        }
        N3: EtherHub {
            parameters:
                @display("p=360,252");
            gates:
                ethg[5];
        }
        N4: EtherHub {
            parameters:
                @display("p=360,464");
            gates:
                ethg[3];
        }
        H1: StandardHost {
            parameters:
                @display("p=56,96;i=device/pc_s");
            gates:
                ethg[1];
        }
        H2: StandardHost {
            parameters:
                @display("p=56,172;i=device/pc_s");
            gates:
                ethg[1];
        }
        H3: StandardHost {
            parameters:
                @display("p=56,316;i=device/pc_s");
            gates:
                ethg[1];
        }
        H4: StandardHost {
            parameters:
                @display("p=56,388;i=device/pc_s");
            gates:
                ethg[1];
        }
        H5: StandardHost {
            parameters:
                @display("p=360,188;i=device/pc_s");
            gates:
                ethg[1];
        }
        H6: StandardHost {
            parameters:
                @display("p=316,520;i=device/pc_s");
            gates:
                ethg[1];
        }
        H7: StandardHost {
            parameters:
                @display("p=400,520;i=device/pc_s");
            gates:
                ethg[1];
        }
    connections:
        H1.ethg[0] <--> C <--> N1.ethg[0];
        N1.ethg[1] <--> C <--> H2.ethg[0];
        N2.ethg[0] <--> C <--> H3.ethg[0];
        N2.ethg[1] <--> C <--> H4.ethg[0];
        N1.ethg[2] <--> C <--> RT1.ethg[0];
        RT1.ethg[1] <--> C <--> N3.ethg[1];
        N3.ethg[0] <--> C <--> H5.ethg[0];
        N4.ethg[0] <--> C <--> H6.ethg[0];
        N4.ethg[1] <--> C <--> H7.ethg[0];
        N2.ethg[2] <--> C <--> RT2.ethg[0];
        RT2.ethg[1] <--> C <--> N3.ethg[2];

        // towards other areas
        N3.ethg[4] <--> { @display("m=m,100,44,100,32"); } <--> ethg[0];
        N3.ethg[3] <--> { @display("m=m,96,96,100,52"); } <--> ethg[1];

        ethg[2] <--> N4.ethg[2];
}

