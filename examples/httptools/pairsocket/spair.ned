
//
// Pair scenario file for the sockets pair example. 
//



package inet.examples.httptools.pairsocket;

import inet.httptools.HttpController;
import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.nodes.inet.StandardHost;


network HttpSockPair
{
    parameters:
        @display("bgb=474,374");
    types:
        channel gigabitline extends ned.DatarateChannel
        {
            parameters:
                delay = 0.1us;
                datarate = 1000Mbps;
        }
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
//                moduleTypes = "StandardHost";
//                nonIPModuleTypes = "";
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("i=block/cogwheel");
        }
        controller: HttpController {
            parameters:
                @display("p=62,60;i=block/cogwheel");
        }
        srv: StandardHost {
            parameters:
                @display("p=170,188;i=device/server_l");
        }
        cli: StandardHost {
            parameters:
                @display("p=316,188;i=device/laptop_l");
        }
    connections:
        cli.ethg++ <--> gigabitline <--> srv.ethg++;
}

