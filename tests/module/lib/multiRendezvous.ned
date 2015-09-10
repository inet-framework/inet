import inet.node.inet.StandardHost;
import inet.transportlayer.sctp.SCTPNatRouter;
import inet.node.inet.Router;
import ned.DatarateChannel;

network multiRendezvous {
    parameters:
        volatile double testTimeout;
        bool testing;
        **.networkLayer.configurator.networkConfiguratorModule = "";
    submodules:
        Peer_1: StandardHost {
            parameters:
                forwarding = false;
        @display("p=56,156;i=device/laptop");
            gates:
                pppg[2];
    }
        Peer_2: StandardHost {
            parameters:
                forwarding = false;
        @display("p=436,160;i=device/laptop");
            gates:
                pppg[2];
    }
        Server: StandardHost {
            parameters:
                forwarding = false;
        @display("p=252,140;i=device/server");
            gates:
                pppg[2];
    }
        Nat_1_1: SCTPNatRouter {
            parameters:
        @display("p=152,104;i=block/join");
            gates:
                pppg[2];
    }
        Nat_2_1: SCTPNatRouter {
            parameters:
        @display("p=340,104;i=block/join");
            gates:
                pppg[2];
    }
        Router_1: Router {
            parameters:
        @display("p=252,48;i=abstract/router");
            gates:
                pppg[3];
    }
        Nat_1_2: SCTPNatRouter {
        parameters:
        @display("p=152,200;i=block/join");
            gates:
                pppg[2];
    }
        Nat_2_2: SCTPNatRouter {
        parameters:
        @display("p=340,204;i=block/join");
            gates:
                pppg[2];
    }
        Router_2: Router {
        parameters:
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

