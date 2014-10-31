import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;

network SCTPFeatureTest
{
    parameters:
        volatile double testTimeout;
        bool testing;
    types:
      channel BottlePath extends DatarateChannel
      {
        parameters:
          datarate = 1Mbps;
          per = 0.005;
      }

      channel NormalPath extends DatarateChannel
      {
        parameters:
          datarate = 1Gbps;
      }
    submodules:
        sctp_client: StandardHost {
            parameters:
                forwarding = false;
                routingTable.routingFile = "../../lib/sctp_client.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
                @display("i=laptop3;p=128,317");
            gates:
                pppg[1];
        }
        sctp_server: StandardHost {
            parameters:
                forwarding = false;
                routingTable.routingFile = "../../lib/sctp_server.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
                @display("i=laptop3");
            gates:
                pppg[1];
        }
        router1: Router {
            parameters:
                routingTable.routingFile = "../../lib/sctp_router1.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
                @display("i=abstract/router;p=270,293");
            gates:
                pppg[2];
        }
        router2: Router {
            parameters:
                routingTable.routingFile = "../../lib/sctp_router2.mrt";
                networkLayer.configurator.networkConfiguratorModule = "";
                @display("i=abstract/router");
            gates:
                pppg[2];
        }
    connections:
        sctp_client.pppg[0] <--> NormalPath <--> router1.pppg[0];
        router2.pppg[0] <--> NormalPath <--> sctp_server.pppg[0];
        router1.pppg[1] <--> BottlePath <--> router2.pppg[1];
}
