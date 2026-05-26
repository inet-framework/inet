package inet.showcases.routing.pim2;

import inet.networks.base.WiredNetworkBase;
import inet.node.ethernet.Eth100M;
import inet.node.inet.MulticastRouter;
import inet.node.inet.StandardHost;

//
// Simple tree topology for demonstrating PIM-DM and PIM-SM.
//
// A multicast source sends traffic through a network of four routers
// to two receivers. R0 is the source's first-hop router. R1 is the
// tree root (and the RP in PIM-SM). R2 and R3 branch to the receivers.
//
network PimShowcaseNetwork extends WiredNetworkBase
{
    submodules:
        source: StandardHost {
            @display("i=device/server;p=400,50");
        }
        R0: MulticastRouter {
            @display("p=400,150");
        }
        R1: MulticastRouter {
            @display("p=400,250");
        }
        R2: MulticastRouter {
            @display("p=250,375");
        }
        R3: MulticastRouter {
            @display("p=550,375");
        }
        receiver1: StandardHost {
            @display("i=device/pc;p=250,500");
        }
        receiver2: StandardHost {
            @display("i=device/pc;p=550,500");
        }
    connections:
        source.ethg++ <--> Eth100M <--> R0.ethg++;
        R0.ethg++ <--> Eth100M <--> R1.ethg++;
        R1.ethg++ <--> Eth100M <--> R2.ethg++;
        R1.ethg++ <--> Eth100M <--> R3.ethg++;
        R2.ethg++ <--> Eth100M <--> receiver1.ethg++;
        R3.ethg++ <--> Eth100M <--> receiver2.ethg++;
}

//
// Larger network for demonstrating an IPTV streaming scenario with PIM-SM.
//
// An IPTV server streams multicast video through a 3x3 grid of routers
// running PIM-SM for multicast. A client joins the multicast group to
// receive the stream.
//
network PimIptvNetwork extends WiredNetworkBase
{
    submodules:
        server: StandardHost {
            @display("i=device/server;p=200,100");
        }
        R00: MulticastRouter {
            @display("p=350,100");
        }
        R01: MulticastRouter {
            @display("p=350,225");
        }
        R02: MulticastRouter {
            @display("p=350,350");
        }
        R10: MulticastRouter {
            @display("p=500,100");
        }
        R11: MulticastRouter {
            @display("p=500,225");
        }
        R12: MulticastRouter {
            @display("p=500,350");
        }
        R20: MulticastRouter {
            @display("p=650,100");
        }
        R21: MulticastRouter {
            @display("p=650,225");
        }
        R22: MulticastRouter {
            @display("p=650,350");
        }
        client: StandardHost {
            @display("i=device/pc;p=800,350");
        }
    connections:
        // Server to edge router
        server.ethg++ <--> Eth100M <--> R00.ethg++;

        // Row connections (horizontal)
        R00.ethg++ <--> Eth100M <--> R10.ethg++;
        R10.ethg++ <--> Eth100M <--> R20.ethg++;
        R01.ethg++ <--> Eth100M <--> R11.ethg++;
        R11.ethg++ <--> Eth100M <--> R21.ethg++;
        R02.ethg++ <--> Eth100M <--> R12.ethg++;
        R12.ethg++ <--> Eth100M <--> R22.ethg++;

        // Column connections (vertical)
        R00.ethg++ <--> Eth100M <--> R01.ethg++;
        R01.ethg++ <--> Eth100M <--> R02.ethg++;
        R10.ethg++ <--> Eth100M <--> R11.ethg++;
        R11.ethg++ <--> Eth100M <--> R12.ethg++;
        R20.ethg++ <--> Eth100M <--> R21.ethg++;
        R21.ethg++ <--> Eth100M <--> R22.ethg++;

        // Client to edge router
        R22.ethg++ <--> Eth100M <--> client.ethg++;
}
