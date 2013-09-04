package inet.examples.ieee802_1d.Bridge1AH;



import ned.DatarateChannel;
import inet.nodes.Ieee8021d.Tester;
import inet.nodes.Ieee8021d.BEB;


//
// Bridge1Q basic testing
//
network BEBTester4
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        Bridge: BEB {
            parameters:
                @display("p=301,118");

            gates:
                ethgB[3];
                ethgC[3];

        }
        T0: Tester {
            parameters:
                @display("p=206,58");
            gates:
                Connection;
        }
        T1: Tester {
            parameters:
                @display("p=301,52");
            gates:
                Connection;
        }
        T2: Tester {
            parameters:
                @display("p=382,58");
            gates:
                Connection;
        }
        TC0: Tester {
            parameters:
                @display("p=206,197");
            gates:
                Connection;
        }
        TC1: Tester {
            parameters:
                @display("p=301,197");
            gates:
                Connection;
        }
        TC2: Tester {
            parameters:
                @display("p=393,197");
            gates:
                Connection;
        }

    connections:
        Bridge.ethgB[0] <--> C <--> T0.Connection;
        Bridge.ethgB[1] <--> C <--> T1.Connection;
        Bridge.ethgB[2] <--> C <--> T2.Connection;
        Bridge.ethgC[0] <--> C <--> TC0.Connection;
        Bridge.ethgC[1] <--> C <--> TC1.Connection;
        Bridge.ethgC[2] <--> C <--> TC2.Connection;
}




//
// BEB RSTP basic testing
//
network BEBNetwork
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        Bridge1: BEB {
            parameters:
                @display("p=274,39");

            gates:
                ethgB[2];

        }
        Bridge2: BEB {
            parameters:
                @display("p=177,111");

            gates:
                ethgB[4];

        }
        Bridge3: BEB {
            parameters:
                @display("p=392,111");

            gates:
                ethgB[3];

        }
        Bridge4: BEB {
            parameters:
                @display("p=448,199");

            gates:
                ethgB[3];

        }
        Bridge5: BEB {
            parameters:
                @display("p=128,199");

            gates:
                ethgB[4];

        }
        Bridge6: BEB {
            parameters:
                @display("p=274,199");

            gates:
                ethgB[4];

        }
        T0: Tester {
            parameters:
                @display("p=79,271");
            gates:
                Connection;
        }
        T1: Tester {
            parameters:
                @display("p=167,271");
            gates:
                Connection;
        }
        T2: Tester {
            parameters:
                @display("p=281,271");
            gates:
                Connection;
        }
        T3: Tester {
            parameters:
                @display("p=448,271");
            gates:
                Connection;
        }
    connections:
        Bridge1.ethgB[0] <--> C <--> Bridge2.ethgB[0];
        Bridge1.ethgB[1] <--> C <--> Bridge3.ethgB[0];
        Bridge2.ethgB[1] <--> C <--> Bridge5.ethgB[0];
        Bridge2.ethgB[2] <--> C <--> Bridge6.ethgB[0];
        Bridge2.ethgB[3] <--> C <--> Bridge3.ethgB[1];
        Bridge3.ethgB[2] <--> C <--> Bridge4.ethgB[0];
        Bridge4.ethgB[1] <--> C <--> Bridge6.ethgB[3];
        Bridge5.ethgB[3] <--> C <--> Bridge6.ethgB[1];

        Bridge5.ethgB[1] <--> C <--> T0.Connection;
        Bridge5.ethgB[2] <--> C <--> T1.Connection;
        Bridge6.ethgB[2] <--> C <--> T2.Connection;
        Bridge4.ethgB[2] <--> C <--> T3.Connection;
}
