package inet.examples.inet.tcp_icmp_error;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;

//
// Demonstrates TCP ICMP error handling (Phase 2 hard error behavior).
//
// The client (StandardHost with TCP) tries to open a TCP connection to the
// target (a Router that has NO TCP module). When the SYN packet arrives at
// the target's IPv4 layer, it cannot deliver it to any transport protocol,
// so it sends back ICMP Destination Unreachable / Protocol Unreachable.
//
// This ICMP error is a "hard" error. With Phase 2 ICMP handling:
//   - TCP recognizes the hard error while in SYN_SENT state
//   - Connection is immediately refused (TCP_I_CONNECTION_REFUSED)
//
// Without Phase 2 (Phase 1 only), the ICMP error would be a soft notification
// and the connection would stay in SYN_SENT until it eventually times out.
//
network TcpIcmpErrorTest
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 100Mbps;
            delay = 1ms;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50;is=s");
        }
        client: StandardHost {
            @display("p=200,150");
        }
        target: Router {
            // Router has hasTcp=false by default (no TCP module).
            // Sending a TCP SYN to this node triggers ICMP protocol unreachable.
            @display("p=400,150");
        }
    connections:
        client.ethg++ <--> C <--> target.ethg++;
}

//
// Demonstrates TCP Path MTU Discovery (Phase 3).
//
// Client sends a large TCP stream to Server through a Router.
// The client-router link has normal MTU (1500), but the router-server
// link has a smaller MTU (configured in ini).
//
// Before Phase 3: TCP does not set the DF (Don't Fragment) bit,
// so IPv4 silently fragments oversized packets at the router.
//
// After Phase 3: TCP sets DF on all segments. When a segment exceeds
// the bottleneck MTU, the router sends ICMP Fragmentation Needed
// (Packet Too Big) with the next-hop MTU. TCP reduces snd_mss
// and retransmits — no IP fragmentation needed.
//
network TcpPmtudTest
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 100Mbps;
            delay = 1ms;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50;is=s");
        }
        client: StandardHost {
            @display("p=100,150");
        }
        router: Router {
            @display("p=300,150");
        }
        server: StandardHost {
            @display("p=500,150");
        }
    connections:
        client.pppg++ <--> C <--> router.pppg++;
        router.pppg++ <--> C <--> server.pppg++;
}
