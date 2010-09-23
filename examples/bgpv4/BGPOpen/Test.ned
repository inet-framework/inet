package inet.examples.bgpv4.BGPOpen;


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
    connections:
        A.pppg++ <--> LINK_100 <--> B.pppg++;
}

