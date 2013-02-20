package inet.examples.bgpv4.BGPOpen;


import inet.base.UnimplementedModule;


network Net
{
    types:
        channel LINK_100 extends ned.DatarateChannel
        {
            parameters:
                delay = 0;
                datarate = 100Mbps;
        }

    submodules:
        A: BGPRouterSimple {
            parameters:
                @display("p=80,60");
        }
        B: BGPRouterSimple {
            parameters:
                @display("p=200,60");
        }
        configurator: UnimplementedModule {
            @display("p=62,127");
        }
    connections:
        A.pppg++ <--> LINK_100 <--> B.pppg++;
}

