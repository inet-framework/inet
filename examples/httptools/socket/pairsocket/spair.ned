//
// Pair scenario file for the sockets pair example.
//

package inet.examples.httptools.socket.pairsocket;

import inet.applications.httptools.configurator.HttpController;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.TransmissionChannel;

network HttpSockPair
{
    parameters:
        *.eth[*].bitrate = 1000 Mbps;
    types:
        channel gigabitline extends TransmissionChannel {
            delay = 0.1us;
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

