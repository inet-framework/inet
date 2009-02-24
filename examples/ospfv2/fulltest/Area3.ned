
package inet.examples.ospfv2.fulltest;

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.OSPFRouter;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


module OSPF_Area3
{
    parameters:
        @display("p=10,10;b=264,528");
    gates:
        inout ethg[];
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
        }
    submodules:
        RT9: OSPFRouter {
            parameters:
                @display("p=148,200");
            gates:
                ethg[2];
        }
        RT12: OSPFRouter {
            parameters:
                @display("p=148,360");
            gates:
                ethg[3];
        }
        N9: EtherHub {
            parameters:
                @display("p=148,284");
            gates:
                ethg[4];
        }
        N10: EtherHub {
            parameters:
                @display("p=148,452");
            gates:
                ethg[3];
        }
        N11: EtherHub {
            parameters:
                @display("p=144,112");
            gates:
                ethg[3];
        }
        H12: StandardHost {
            parameters:
                @display("p=92,52;i=device/pc_s");
            gates:
                ethg[1];
        }
        H13: StandardHost {
            parameters:
                @display("p=196,52;i=device/pc_s");
            gates:
                ethg[1];
        }
        H14: StandardHost {
            parameters:
                @display("p=60,284;i=device/pc_s");
            gates:
                ethg[1];
        }
        H15: StandardHost {
            parameters:
                @display("p=60,360;i=device/pc_s");
            gates:
                ethg[1];
        }
        H16: StandardHost {
            parameters:
                @display("p=92,492;i=device/pc_s");
            gates:
                ethg[1];
        }
        H17: StandardHost {
            parameters:
                @display("p=196,492;i=device/pc_s");
            gates:
                ethg[1];
        }
    connections:
        H12.ethg[0] <--> C <--> N11.ethg[0];
        N11.ethg[1] <--> C <--> RT9.ethg[0];
        N11.ethg[2] <--> C <--> H13.ethg[0];
        RT9.ethg[1] <--> C <--> N9.ethg[0];
        N9.ethg[1] <--> C <--> H14.ethg[0];
        N9.ethg[2] <--> C <--> RT12.ethg[0];
        RT12.ethg[1] <--> C <--> H15.ethg[0];
        RT12.ethg[2] <--> C <--> N10.ethg[0];
        N10.ethg[1] <--> C <--> H16.ethg[0];
        N10.ethg[2] <--> C <--> H17.ethg[0];

        // towards other areas
        ethg[0] <--> N9.ethg[3];
}

