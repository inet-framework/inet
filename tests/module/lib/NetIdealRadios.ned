
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.AdhocHost;
import inet.world.radio.IdealChannelModel;


network NetIdealRadios
{
    parameters:
        int numHosts;
        @display("bgb=600,400");
    submodules:
        host[numHosts]: AdhocHost {
            parameters:
                @display("r=,,#707070;p=250,58");
        }
        channelControl: IdealChannelModel {
            parameters:
                @display("p=60,50");
        }
        configurator: IPv4NetworkConfigurator {
            config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.0.0'/></config>");
            @display("p=140,50");
        }
}

