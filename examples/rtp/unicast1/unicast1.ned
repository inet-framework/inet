

package inet.examples.rtp.unicast1;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.rtp.RTPHost;
import ned.DatarateChannel;


network RTPUnicast1
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
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RTPAVProfile";
                destinationAddress = "host4";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=150,50");
        }
        host2: RTPHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RTPAVProfile";
                destinationAddress = "host4";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=250,150");
        }
        host3: RTPHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RTPAVProfile";
                destinationAddress = "host4";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=150,250");
        }
        host4: RTPHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RTPAVProfile";
                destinationAddress = "host1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=50,150");
        }
        router1: Router {
            parameters:
                @display("p=150,150");
            gates:
                pppg[];

        }
        configurator: IPv4NetworkConfigurator {
            @display("p=58,32");
        }
    connections:
        host1.pppg++ <--> ethernet <--> router1.pppg++;
        host2.pppg++ <--> ethernet <--> router1.pppg++;
        host3.pppg++ <--> ethernet <--> router1.pppg++;
        host4.pppg++ <--> ethernet <--> router1.pppg++;
}
