package inet.examples.bgpv4.BGPOpen;

import BGPRouterSimple;

channel LINK_100 extends ned.DatarateChannel // 100 Gbps
{
    parameters:
        delay = 0;
        datarate = 100e9bps;
    //error 1
}

module Test
{
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

network Net extends Test
{
    parameters:
}

