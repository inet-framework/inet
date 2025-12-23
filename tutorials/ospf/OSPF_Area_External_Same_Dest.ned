package inet.tutorials.ospf;

import inet.node.ethernet.Eth100M;


network OSPF_Area_External_Same_Dest extends OSPF_AreaTest
{
    @display("bgb=1151.665,517.31");
    submodules:
        N1: OspfLan {
            parameters:
                h = 1;
                @display("p=539.45123,389.04126");
        }

    connections:
        R3.ethg++ <--> Eth100M <--> N1.ethg++;
        R5.ethg++ <--> Eth100M <--> N1.ethg++;
}

