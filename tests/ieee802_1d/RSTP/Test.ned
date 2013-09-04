package inet.examples.ieee802_1d.RSTP;



import ned.DatarateChannel;
import inet.nodes.Ieee8021d.Tester;
import inet.nodes.Ieee8021d.BEB;


//
// BEB RSTP basic testing
//
network BEB1Tester4
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        BEB1: BEB {
            parameters:
                @display("p=301,118");

            gates:
                ethgB[4];

        }
        T0: Tester {
            parameters:
                @display("p=301,48");
            gates:
                Connection;
        }
        T1: Tester {
            parameters:
                @display("p=192,118");
            gates:
                Connection;
        }
        T2: Tester {
            parameters:
                @display("p=301,201");
            gates:
                Connection;
        }
        T3: Tester {
            parameters:
                @display("p=411,118");
            gates:
                Connection;
        }
    connections:
        BEB1.ethgB[0] <--> C <--> T0.Connection;
        BEB1.ethgB[1] <--> C <--> T1.Connection;
        BEB1.ethgB[2] <--> C <--> T2.Connection;
        BEB1.ethgB[3] <--> C <--> T3.Connection;

}


//
// A complete network without clients. Testing priority mechanism.
//
network CompleteNetwork
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        switchBEB1: BEB {
            parameters:
                @display("p=333,78");

            gates:
                ethgB[2];

        }
        switchBEB2: BEB {
            parameters:
                @display("p=301,217");

            gates:
                ethgB[3];
        }
        switchBEB3: BEB {
            parameters:
                @display("p=443,78");


            gates:
                ethgB[2];

        }
        switchBEB4: BEB {
            parameters:
                @display("p=322,153");

            gates:
                ethgB[4];

        }
        switchBEB5: BEB {
            parameters:
                @display("p=529,217");

            gates:
                ethgB[1];

        }
        switchBEB6: BEB {
            parameters:
                @display("p=148,217");

            gates:
                ethgB[2];

        }
        switchBEB7: BEB {
            parameters:
                @display("p=443,153");

            gates:
                ethgB[4];

        }
    connections:
        switchBEB1.ethgB[0] <--> C <--> switchBEB4.ethgB[0];
        switchBEB1.ethgB[1] <--> C <--> switchBEB3.ethgB[0];
        switchBEB2.ethgB[0] <--> C <--> switchBEB4.ethgB[2];
        switchBEB2.ethgB[1] <--> C <--> switchBEB6.ethgB[1];
        switchBEB2.ethgB[2] <--> C <--> switchBEB7.ethgB[2];
        switchBEB3.ethgB[1] <--> C <--> switchBEB7.ethgB[0];
        switchBEB4.ethgB[1] <--> C <--> switchBEB6.ethgB[0];
        switchBEB4.ethgB[3] <--> C <--> switchBEB7.ethgB[1];
        switchBEB5.ethgB[0] <--> C <--> switchBEB7.ethgB[3];
}

//
// A complete network with a backup link and an alternate link with the same source. Testing priority mechanism.
//  7====5   and 2=autoconnected
//
network NetworkWithBK
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;            
        }
    submodules:
        switchBEB1: BEB {
            parameters:
                @display("p=333,78");

            gates:
                ethgB[2];

        }
        switchBEB2: BEB {
            parameters:
                @display("p=301,217");

            gates:
                ethgB[5];
        }
        switchBEB3: BEB {
            parameters:
                @display("p=443,78");


            gates:
                ethgB[2];

        }
        switchBEB4: BEB {
            parameters:
                @display("p=322,153");

            gates:
                ethgB[4];
        }
        switchBEB5: BEB {
            parameters:
                @display("p=529,217");

            gates:
                ethgB[2];
        }
        switchBEB6: BEB {
            parameters:
                @display("p=148,217");

            gates:
                ethgB[2];
        }
        switchBEB7: BEB {
            parameters:
                @display("p=443,153");

            gates:
                ethgB[5];

        }
    connections:
        switchBEB1.ethgB[0] <--> C <--> switchBEB4.ethgB[0];
        switchBEB1.ethgB[1] <--> C <--> switchBEB3.ethgB[0];
        switchBEB2.ethgB[0] <--> C <--> switchBEB4.ethgB[2];
        switchBEB2.ethgB[1] <--> C <--> switchBEB6.ethgB[1];
        switchBEB2.ethgB[2] <--> C <--> switchBEB7.ethgB[2];
        switchBEB3.ethgB[1] <--> C <--> switchBEB7.ethgB[0];
        switchBEB4.ethgB[1] <--> C <--> switchBEB6.ethgB[0];
        switchBEB4.ethgB[3] <--> C <--> switchBEB7.ethgB[1];
        switchBEB5.ethgB[0] <--> C <--> switchBEB7.ethgB[3];
        switchBEB5.ethgB[1] <--> C <--> switchBEB7.ethgB[4];
        switchBEB2.ethgB[3] <--> C <--> switchBEB2.ethgB[4];
}
