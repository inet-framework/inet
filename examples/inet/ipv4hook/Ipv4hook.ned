//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.inet.ipv4hook;

import inet.networklayer.common.NetfilterInfoHook;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


module MyHost extends StandardHost
{
    submodules:
        iPv4HookInfo: NetfilterInfoHook {
            networkProtocolModule = "^.ipv4.ip";
            @display("p=225,80");
        }
}

module MyRouter extends Router
{
    submodules:
        iPv4HookInfo: NetfilterInfoHook {
            networkProtocolModule = "^.ipv4.ip";
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
            @display("p=250,70");
        }
        host2: MyHost {
            @display("p=450,70");
        }
        router: MyRouter {
            @display("p=350,170");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,70;is=s");
        }
    connections:
        host1.ethg++ <--> C <--> router.ethg++;
        host2.ethg++ <--> C <--> router.ethg++;
}
