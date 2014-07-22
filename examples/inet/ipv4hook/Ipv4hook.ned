//
// This program is property of its copyright holder. All rights reserved.
//

package inet.examples.inet.ipv4hook;

import inet.networklayer.common.NetfilterInfoHook;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


module MyHost extends StandardHost
{
    submodules:
        iPv4HookInfo: NetfilterInfoHook {
            @display("p=626,265");
        }
}

module MyRouter extends Router
{
    submodules:
        iPv4HookInfo: NetfilterInfoHook {
            @display("p=524,98");
        }
}


//
// TODO Auto-generated network
//
network Ipv4hook
{
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        host1: MyHost {
            @display("p=43,70");
        }
        host2: MyHost {
            @display("p=271,70");
        }
        router: MyRouter {
            @display("p=157,197");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=112,278");
        }
    connections:
        host1.ethg++ <--> C <--> router.ethg++;
        host2.ethg++ <--> C <--> router.ethg++;
}
