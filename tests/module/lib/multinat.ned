package inet.test.moduletest.lib;

import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.transportlayer.sctp.SctpNatRouter;


network multinat
{
    parameters:
        volatile double testTimeout @unit(s);
        bool testing;
        **.ipv4.configurator.networkConfiguratorModule = "";
    submodules:
        Peer_1: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "../../lib/nat_peer1.mrt";
        @display("p=48,124;i=device/laptop");
            gates:
                pppg[2];
    }
        Server: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "../../lib/nat_server.mrt";
        @display("p=452,116;i=device/server");
            gates:
                pppg[2];
    }
        Nat_1_1: SctpNatRouter {
            parameters:
                ipv4.routingTable.routingFile = "../../lib/nat1_1.mrt";
        @display("p=152,56;i=block/join");
            gates:
                pppg[2];
    }
        Router_1: Router {
            parameters:
                ipv4.routingTable.routingFile = "../../lib/nat_mrouter_1.mrt";
        @display("p=300,56;i=abstract/router");
            gates:
                pppg[2];
    }
        Nat_1_2: SctpNatRouter {
        parameters:
                ipv4.routingTable.routingFile = "../../lib/nat1_2.mrt";
        @display("p=156,172;i=block/join");
            gates:
                pppg[2];
    }
        Router_2: Router {
        parameters:
                ipv4.routingTable.routingFile = "../../lib/nat_mrouter_2.mrt";
        @display("p=316,172;i=abstract/router");
            gates:
                pppg[2];
    }
    connections:
        Peer_1.pppg[0] <--> natline <--> Nat_1_1.pppg[0];

        Nat_1_1.pppg[1] <--> natline <--> Router_1.pppg[0];

        Router_1.pppg[1] <--> natline <--> Server.pppg[0];

        Peer_1.pppg[1] <--> natline <--> Nat_1_2.pppg[0];

        Nat_1_2.pppg[1] <--> natline <--> Router_2.pppg[0];

        Router_2.pppg[1] <--> natline <--> Server.pppg[1];

}


