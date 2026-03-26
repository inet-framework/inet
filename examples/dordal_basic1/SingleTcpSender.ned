//
// Recreates the ns-2 basic1.tcl topology from Dordal Ch.31.2:
//   A ---[10Mbps/10ms]--- R ---[800Kbps/50ms]--- B
// R-B is the bottleneck link (100 packets/sec with 1000-byte packets).
// Bandwidth x RTT_noLoad = 0.1 pkt/ms x 120 ms = 12 packets.
//

package inet.examples.dordal_basic1;

import inet.networks.base.WiredNetworkBase;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;

network SingleTcpSender extends WiredNetworkBase
{
    @display("bgb=600,300");
    submodules:
        A: StandardHost {
            @display("p=100,150");
        }
        R: Router {
            @display("p=300,150");
        }
        B: StandardHost {
            @display("p=500,150");
        }
    connections:
        A.pppg++ <--> { datarate = 10Mbps; delay = 10ms; } <--> R.pppg++;
        R.pppg++ <--> { datarate = 800kbps; delay = 50ms; } <--> B.pppg++;
}
