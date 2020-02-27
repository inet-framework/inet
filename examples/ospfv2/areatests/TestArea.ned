
package inet.examples.ospfv2.areatests;

import inet.node.ethernet.EtherSwitch;
import inet.node.inet.StandardHost;
import inet.node.ospfv2.OspfRouter;
import ned.TransmissionChannel;


module TestArea
{
    parameters:
        @display("p=10,10;b=412,316");
        int numHosts = default(3);
        int extGates = default(0);
        *.eth[*].bitrate = 100 Mbps;
    gates:
        inout ethg[extGates];
    types:
        channel C extends TransmissionChannel
        {
            delay = 0.1us;
        }
    submodules:
        H[numHosts]: StandardHost {
            @display("i=device/laptop;p=100,400,row,100");
        }
        N[numHosts]: EtherSwitch {
            @display("p=100,300,row,100");
        }
        R[numHosts]: OspfRouter {
            @display("p=100,200,row,100");
        }
        B[extGates]: OspfRouter {
            @display("p=100,100,row,100");
        }
    connections:
        // host to router links
        for i=0..numHosts-1 {
            H[i].ethg++ <--> C <--> N[i].ethg++;
            R[i].ethg++ <--> C <--> N[i].ethg++;
        }

        // towards other areas
        for i=0..extGates-1 {
            ethg[i] <--> B[i].ethg++;
        }

        // ring of routers
        for i=1..numHosts-1 {
            R[i].ethg++ <--> C <--> R[i-1].ethg++;
        }
        for i=1..extGates-1 {
            B[i].ethg++ <--> C <--> B[i-1].ethg++;
        }
        R[0].ethg++ <--> C <--> R[numHosts-1].ethg++ if numHosts > 2 && extGates == 0;
        B[0].ethg++ <--> C <--> B[extGates-1].ethg++ if numHosts == 0 && extGates > 2;
        R[0].ethg++ <--> C <--> B[extGates-1].ethg++ if numHosts > 0 && extGates > 0;
        B[0].ethg++ <--> C <--> R[numHosts-1].ethg++ if numHosts > 0 && extGates > 0 && (numHosts+extGates > 2);
}
