
package inet.examples.ospfv2.fulltest;

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.OSPFRouter;
import inet.nodes.inet.StandardHost;


module OSPF_Area2
{
    parameters:
        @display("p=10,10;b=424,400");
    gates:
        inout ethg[];
    submodules:
        RT8: OSPFRouter {
            parameters:
                @display("p=304,196");
            gates:
                ethg[2];
        }
        N6: EtherHub {
            parameters:
                @display("p=304,100");
            gates:
                ethg[4];
        }
        N7: EtherHub {
            parameters:
                @display("p=304,300");
            gates:
                ethg[3];
        }
        N8: EtherHub {
            parameters:
                @display("p=104,100");
            gates:
                ethg[3];
        }
        H8: StandardHost {
            parameters:
                @display("p=392,100;i=device/pc_s");
            gates:
                ethg[1];
        }
        H9: StandardHost {
            parameters:
                @display("p=348,364;i=device/pc_s");
            gates:
                ethg[1];
        }
        H10: StandardHost {
            parameters:
                @display("p=260,364;i=device/pc_s");
            gates:
                ethg[1];
        }
        H11: StandardHost {
            parameters:
                @display("p=104,156;i=device/pc_s");
            gates:
                ethg[1];
        }
    connections:
        N8.ethg[1] <--> {  delay = 0.1us; } <--> H11.ethg[0];
        N6.ethg[2] <--> {  delay = 0.1us; } <--> RT8.ethg[0];
        N6.ethg[3] <--> {  delay = 0.1us; } <--> H8.ethg[0];
        RT8.ethg[1] <--> {  delay = 0.1us; } <--> N7.ethg[0];
        N7.ethg[1] <--> {  delay = 0.1us; } <--> H10.ethg[0];
        N7.ethg[2] <--> {  delay = 0.1us; } <--> H9.ethg[0];

        // towards other areas
        ethg[0] <--> N8.ethg[0];
        N8.ethg[2] <--> { @display("m=m,52,64,40,0"); } <--> ethg[1];
        N6.ethg[1] <--> { @display("m=m,28,36,48,0"); } <--> ethg[2];
        ethg[3] <--> N6.ethg[0];
}

