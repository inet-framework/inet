
package inet.examples.ospfv2.areatests;

import inet.linklayer.ethernet.EtherHub;
import inet.nodes.inet.StandardHost;
import inet.nodes.ospfv2.OSPFRouter;
import inet.util.ThruputMeteringChannel;


module TestArea
{
    parameters:
        @display("p=10,10;b=412,316");
        int numHosts = default(3);
        int extGates = default(0);
    gates:
        inout ethg[extGates];
    types:
        channel C extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
            thruputDisplayFormat = "#N";
        }
    submodules:
        H[numHosts]: StandardHost {
            @display("i=device/laptop");
        }
        N[numHosts]: EtherHub;
        R[numHosts]: OSPFRouter;
        B[extGates]: OSPFRouter;
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
