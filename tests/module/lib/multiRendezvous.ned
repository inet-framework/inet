package inet.test.moduletest.lib;

import inet.node.inet.StandardHost;
import inet.transportlayer.sctp.SctpNatRouter;
import inet.node.inet.Router;
import ned.DatarateChannel;

network multiRendezvous {
    parameters:
        volatile double testTimeout @unit(s);
        bool testing;
        **.ipv4.configurator.networkConfiguratorModule = "";
    submodules:
        Peer_1: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "../../lib/nat_peer1.mrt";
        @display("p=56,156;i=device/laptop");
            gates:
                pppg[2];
    }
        Peer_2: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "../../lib/nat_peer2.mrt";
        @display("p=436,160;i=device/laptop");
            gates:
                pppg[2];
    }
        Server: StandardHost {
            parameters:
                forwarding = false;
                ipv4.routingTable.routingFile = "../../lib/nat_server.mrt";
        @display("p=252,140;i=device/server");
            gates:
                pppg[2];
    }
        Nat_1_1: SctpNatRouter {
            parameters:
            ipv4.routingTable.routingFile = "../../lib/nat1_1.mrt";
        @display("p=152,104;i=block/join");
            gates:
                pppg[2];
    }
        Nat_2_1: SctpNatRouter {
            parameters:
            ipv4.routingTable.routingFile = "../../lib/nat2_1.mrt";
        @display("p=340,104;i=block/join");
            gates:
                pppg[2];
    }
        Router_1: Router {
            parameters:
            ipv4.routingTable.routingFile = "../../lib/nat_router_1.mrt";
        @display("p=252,48;i=abstract/router");
            gates:
                pppg[3];
    }
        Nat_1_2: SctpNatRouter {
        parameters:
        ipv4.routingTable.routingFile = "../../lib/nat1_2.mrt";
        @display("p=152,200;i=block/join");
            gates:
                pppg[2];
    }
        Nat_2_2: SctpNatRouter {
        parameters:
        ipv4.routingTable.routingFile = "../../lib/nat2_2.mrt";
        @display("p=340,204;i=block/join");
            gates:
                pppg[2];
    }
        Router_2: Router {
        parameters:
        ipv4.routingTable.routingFile = "../../lib/nat_router_2.mrt";
        @display("p=252,232;i=abstract/router");
            gates:
                pppg[3];
    }
    connections:
        Peer_1.pppg[0] <--> natline <--> Nat_1_1.pppg[0];
        Nat_1_1.pppg[1] <--> natline <--> Router_1.pppg[0];
        Router_1.pppg[2] <--> natline <--> Server.pppg[0];
        Router_1.pppg[1] <--> natline <--> Nat_2_1.pppg[0];
        Nat_2_1.pppg[1] <--> natline <--> Peer_2.pppg[0];
        Peer_1.pppg[1] <--> natline <--> Nat_1_2.pppg[0];
        Nat_2_2.pppg[1] <--> natline <--> Peer_2.pppg[1];
        Nat_1_2.pppg[1] <--> natline <--> Router_2.pppg[0];
        Router_2.pppg[1] <--> natline <--> Nat_2_2.pppg[0];
        Router_2.pppg[2] <--> natline <--> Server.pppg[1];

}

channel natline extends DatarateChannel
{
    datarate = 100 Mbps;
}

