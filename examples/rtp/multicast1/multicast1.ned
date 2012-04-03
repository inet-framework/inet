

package inet.examples.rtp.multicast1;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
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
                IPForward = false;
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=150,50");
        }
        host2: RTPHost {
            parameters:
                IPForward = false;
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=250,150");
        }
        host3: RTPHost {
            parameters:
                IPForward = false;
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=150,250");
        }
        host4: RTPHost {
            parameters:
                IPForward = false;
                profileName = "inet.transport.rtp.RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=50,150");
        }
        router1: Router {
            parameters:
                forwardMulticast = true;
                @display("p=150,150");
            gates:
                pppg[];

        }
        configurator: IPv4NetworkConfigurator {
            config = xml("<config>"
                +"<interface hosts='**' address='10.x.x.x' netmask='255.x.x.x'/>"
                +"<multicast-group hosts='**' address='225.0.0.1'/>"
                +"<multicast-route hosts='router1' groups='225.0.0.1' children='ppp*'/>"
                +"</config>");
        }
    connections:
        host1.pppg++ <--> ethernet <--> router1.pppg++;
        host2.pppg++ <--> ethernet <--> router1.pppg++;
        host3.pppg++ <--> ethernet <--> router1.pppg++;
        host4.pppg++ <--> ethernet <--> router1.pppg++;
}
