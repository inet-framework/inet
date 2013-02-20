package inet.examples.bgpv4.BGPOpen;


import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;


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
        configurator: IPv4NetworkConfigurator {
            @display("p=62,127");
            config = xmldoc("IPv4Config.xml");
            //assignAddresses = false;
            //assignDisjunctSubnetAddresses = false;
            addStaticRoutes = false;
            addDefaultRoutes = false;
            addSubnetRoutes = false;
        }
    connections:
        A.pppg++ <--> LINK_100 <--> B.pppg++;
}

