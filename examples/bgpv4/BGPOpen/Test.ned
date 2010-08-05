package inet.examples.bgpv4.BGPOpen;

import BGPRouterSimple;


network Net
{
    types:
        channel LINK_100 extends ned.DatarateChannel
        {
            parameters:
                delay = 0;
                datarate = 100Gbps;
        }

    submodules:
        A: BGPRouterSimple {
            parameters:
                @display("p=80,60;i=cisco/router");
            gates:
                pppIn[1];
                pppOut[1];
        }
        B: BGPRouterSimple {
            parameters:
                @display("p=200,60;i=cisco/router");
            gates:
                pppIn[1];
                pppOut[1];
        }
    connections:
        A.pppOut[0] --> LINK_100 --> B.pppIn[0];
        B.pppOut[0] --> LINK_100 --> A.pppIn[0];
}

