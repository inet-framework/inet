import ned.DatarateChannel;
import inet.nodes.inet.StandardHost;
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;

network SimpleTestNetwork
{
  types:
    channel pppline extends DatarateChannel
    {
       delay = 10ms;
       datarate = 10Mbps;
    }
  submodules:
    client: StandardHost;
    server: StandardHost;
    configurator: IPv4NetworkConfigurator;
  connections:
    client.pppg++ <--> pppline <--> server.pppg++;        
}
