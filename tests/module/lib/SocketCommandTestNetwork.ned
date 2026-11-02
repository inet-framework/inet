package inet.test.moduletest.lib;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;

//
// Two hosts on one Ethernet link, for testing socket commands that reach a
// protocol layer without socket support. See socket-command.ini.
//
network SocketCommandTestNetwork
{
    submodules:
        configurator: Ipv4NetworkConfigurator;
        host1: StandardHost;
        host2: StandardHost;
    connections:
        host1.ethg++ <--> Eth100M <--> host2.ethg++;
}
