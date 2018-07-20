

package inet.examples.rtp.multicast1;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.rtp.RtpHost;
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
        host1: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=400,100");
        }
        host2: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=550,230");
        }
        host3: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=400,360");
        }
        host4: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=250,230");
        }
        router1: Router {
            parameters:
                multicastForwarding = true;
                @display("p=400,230");
            gates:
                pppg[];

        }
        configurator: Ipv4NetworkConfigurator {
            config = xml("<config>"
                +"<interface hosts='**' address='10.x.x.x' netmask='255.x.x.x'/>"
                +"<multicast-group hosts='**' address='225.0.0.1'/>"
                +"<multicast-route hosts='router1' groups='225.0.0.1' children='ppp*'/>"
                +"</config>");
            @display("p=100,100;is=s");
        }
    connections:
        host1.pppg++ <--> ethernet <--> router1.pppg++;
        host2.pppg++ <--> ethernet <--> router1.pppg++;
        host3.pppg++ <--> ethernet <--> router1.pppg++;
        host4.pppg++ <--> ethernet <--> router1.pppg++;
}
