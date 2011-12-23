package inet.tests.ethernet;

import inet.base.DurationRecorder;
import inet.linklayer.ethernet.EtherHub;
import inet.nodes.ethernet.EtherHost2;
import ned.DatarateChannel;


network NHosts
{
    parameters:
        int n;
    submodules:
        host[n]: EtherHost2 {
            @display("p=72,80");
            app.destAddress = "host[" + string((index+1)%n) + "]";
        }
        hub: EtherHub {
            @display("p=213,53");
        }
        d : DurationRecorder;
    connections:
        for i=0..n-1 {
            host[i].ethg <--> DatarateChannel <--> hub.ethg++;
        }
}
