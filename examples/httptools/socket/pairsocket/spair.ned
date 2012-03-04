//
// Pair scenario file for the sockets pair example.
//



package inet.examples.httptools.socket.pairsocket;

import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.StandardHost;
import inet.world.httptools.HttpController;


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
        configurator: IPv4NetworkConfigurator {
            parameters:
                //                moduleTypes = "StandardHost";
                //                nonIPModuleTypes = "";
                config=xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
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

