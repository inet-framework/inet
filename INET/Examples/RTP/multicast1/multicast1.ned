

package org.omnetpp.inet.examples.RTP.Multicast1;

import ned.cBasicChannel;
import org.omnetpp.inet.Nodes.INET.Router;
import org.omnetpp.inet.Transport.RTP.RTPHost;


network RTPMulticast1
{
    parameters:
        bool debug = default(false);
    types:
        channel ethernet extends cBasicChannel
        {
            delay = normal(0.00015,0.00005);
            datarate = 10Mbps;
        }
    submodules:
        host1: RTPHost {
            parameters:
                debug = debug;
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                numOfProcessors = 1;
                routingFile = "host1.mrt";
                @display("p=150,50");
        }
        host2: RTPHost {
            parameters:
                debug = debug;
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                numOfProcessors = 1;
                routingFile = "host2.mrt";
                @display("p=250,150");
        }
        host3: RTPHost {
            parameters:
                debug = debug;
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                numOfProcessors = 1;
                routingFile = "host3.mrt";
                @display("p=150,250");
        }
        host4: RTPHost {
            parameters:
                debug = debug;
                profileName = "RTPAVProfile";
                destinationAddress = "225.0.0.1";
                portNumber = 5004;
                bandwidth = 8000;
                numOfProcessors = 1;
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

