//
// Pair scenario file for the sockets pair example.
//



package inet.examples.httptools.socket.pairsocket;

import inet.applications.httptools.configurator.HttpController;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;


network HttpSockPair
{
    parameters:
    types:
        channel gigabitline extends ned.DatarateChannel
        {
            parameters:
                delay = 0.1us;
                datarate = 1000Mbps;
        }
    submodules:
        configurator: Ipv4NetworkConfigurator {
            parameters:
                //                moduleTypes = "StandardHost";
                //                nonIPModuleTypes = "";
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("is=s;p=100,100");
        }
        httpController: HttpController {
            parameters:
                @display("is=s;p=100,200;i=block/cogwheel");
        }
        srv: StandardHost {
            parameters:
                @display("p=250,150;i=device/server");
        }
        cli: StandardHost {
            parameters:
                @display("p=400,150;i=device/laptop");
        }
    connections:
        cli.ethg++ <--> gigabitline <--> srv.ethg++;
}

