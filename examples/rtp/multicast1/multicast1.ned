

package inet.examples.rtp.multicast1;

import inet.nodes.inet.Router;
import inet.transport.rtp.RTPHost;
import ned.DatarateChannel;


network RTPMulticast1
{
    types:
        channel ethernet extends DatarateChannel
        {
            delay = normal(0.00015s, 0.00005s);
            datarate = 10Mbps;
        }
    submodules:
        host1: RTPHost {
            parameters:
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                routingFile = "host1.mrt";
                @display("p=150,50");
        }
        host2: RTPHost {
            parameters:
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                routingFile = "host2.mrt";
                @display("p=250,150");
        }
        host3: RTPHost {
            parameters:
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                routingFile = "host3.mrt";
                @display("p=150,250");
        }
        host4: RTPHost {
            parameters:
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                routingFile = "host4.mrt";
                @display("p=50,150");
        }
        router1: Router {
            parameters:
                routingFile = "router1.mrt";
                @display("p=150,150");
            gates:
                pppg[4];

        }
    connections:
        host1.pppg[0] <--> ethernet <--> router1.pppg[0];
        host2.pppg[0] <--> ethernet <--> router1.pppg[1];
        host3.pppg[0] <--> ethernet <--> router1.pppg[2];
        host4.pppg[0] <--> ethernet <--> router1.pppg[3];

}

