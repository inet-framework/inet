//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
