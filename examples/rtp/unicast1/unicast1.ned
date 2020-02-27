

package inet.examples.rtp.unicast1;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.rtp.RtpHost;
import ned.DatarateChannel;


network RTPUnicast1
{
    types:
        channel pppline extends DatarateChannel
        {
            delay = normal(0.00015s, 0.00005s);
            datarate = 10Mbps;
        }
    submodules:
        host1: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "host4";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=400,100");
        }
        host2: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "host4";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=550,230");
        }
        host3: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "host4";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=400,360");
        }
        host4: RtpHost {
            parameters:
                forwarding = false;
                profileName = "inet.transportlayer.rtp.RtpAvProfile";
                destinationAddress = "host1";
                portNumber = 5004;
                bandwidth = 8000;
                @display("p=250,230");
        }
        router1: Router {
            parameters:
                @display("p=400,230");
            gates:
                pppg[];

        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
    connections:
        host1.pppg++ <--> pppline <--> router1.pppg++;
        host2.pppg++ <--> pppline <--> router1.pppg++;
        host3.pppg++ <--> pppline <--> router1.pppg++;
        host4.pppg++ <--> pppline <--> router1.pppg++;
}
