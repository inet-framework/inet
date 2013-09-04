package inet.examples.ieee802_1d.MVRP;

import inet.nodes.Ieee8021d.BEB;
import inet.nodes.Ieee8021d.EthernetHost;		//EtherHost. Hosts modificados para que generen tramas 802.1Q y esas necesidades.
import ned.DatarateChannel;
import inet.nodes.Ieee8021d.Tester;



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
// A complete network without clients. Testing MVRP
//
network DiffusionTreeTest
{
    @display("bgb=689,431");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        BEB1: BEB {
            parameters:
                @display("p=333,124");

            gates:
                ethgB[4];

        }
        BEB2: BEB {
            parameters:
                @display("p=237,180");

            gates:
                ethgB[3];
        }
        BEB3: BEB {
            parameters:
                @display("p=443,180");
            gates:
                ethgB[3];

        }
        BEB4: BEB {
            parameters:
                @display("p=167,277");
            gates:
                ethgB[2];
        }
        BEB5: BEB {
            parameters:
                @display("p=283,277");

            gates:
                ethgB[2];
        }
        BEB6: BEB {
            parameters:
                @display("p=397,277");
            gates:
                ethgB[2];
        }
        BEB7: BEB {
            parameters:
                @display("p=513,277");
            gates:
                ethgB[2];
        }
        T0: Tester {
            parameters:
                @display("p=283,27");
        }
        T1: Tester {
            parameters:
                @display("p=372,27");
        }
        T2: Tester {
            parameters:
                @display("p=167,376");
        }
        T3: Tester {
            parameters:
                @display("p=283,376");
        }
        T4: Tester {
            parameters:
                @display("p=397,376");
        }
        T5: Tester {
            parameters:
                @display("p=513,376");
        }

    connections:
        BEB1.ethgB[0] <--> C <--> BEB2.ethgB[0];
        BEB1.ethgB[1] <--> C <--> BEB3.ethgB[0];
        BEB2.ethgB[1] <--> C <--> BEB4.ethgB[0];
        BEB2.ethgB[2] <--> C <--> BEB5.ethgB[0];
        BEB3.ethgB[1] <--> C <--> BEB6.ethgB[0];
        BEB3.ethgB[2] <--> C <--> BEB7.ethgB[0];

        BEB1.ethgB[2] <--> C <--> T0.Connection;
        BEB1.ethgB[3] <--> C <--> T1.Connection;
        BEB4.ethgB[1] <--> C <--> T2.Connection;
        BEB5.ethgB[1] <--> C <--> T3.Connection;
        BEB6.ethgB[1] <--> C <--> T4.Connection;
        BEB7.ethgB[1] <--> C <--> T5.Connection;
}
