package inet.examples.ieee802_1d.Bridge1AD;



import ned.DatarateChannel;
import inet.nodes.Ieee8021d.Tester;
import inet.nodes.Ieee8021d.Ether1ADBridge;
import inet.nodes.Ieee8021d.EthernetHost;


//
// Bridge1Q basic testing
//
network Bridge1ADTester4
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        Bridge: Ether1ADBridge {
            parameters:
                @display("p=301,118");

            gates:
                ethg[4];

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
        Bridge.ethg[0] <--> C <--> T0.Connection;
        Bridge.ethg[1] <--> C <--> T1.Connection;
        Bridge.ethg[2] <--> C <--> T2.Connection;
        Bridge.ethg[3] <--> C <--> T3.Connection;

}




//
// Bridge 1AD basic testing (with Tester)
//
network Bridge1ADNetwork
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        Bridge1: Ether1ADBridge {
            parameters:
                @display("p=274,39");

            gates:
                ethg[2];

        }
        Bridge2: Ether1ADBridge {
            parameters:
                @display("p=177,111");

            gates:
                ethg[4];

        }
        Bridge3: Ether1ADBridge {
            parameters:
                @display("p=392,111");

            gates:
                ethg[3];

        }
        Bridge4: Ether1ADBridge {
            parameters:
                @display("p=448,199");

            gates:
                ethg[3];

        }
        Bridge5: Ether1ADBridge {
            parameters:
                @display("p=128,199");

            gates:
                ethg[4];

        }
        Bridge6: Ether1ADBridge {
            parameters:
                @display("p=274,199");

            gates:
                ethg[4];

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
        Bridge1.ethg[0] <--> C <--> Bridge2.ethg[0];
        Bridge1.ethg[1] <--> C <--> Bridge3.ethg[0];
        Bridge2.ethg[1] <--> C <--> Bridge5.ethg[0];
        Bridge2.ethg[2] <--> C <--> Bridge6.ethg[0];
        Bridge2.ethg[3] <--> C <--> Bridge3.ethg[1];
        Bridge3.ethg[2] <--> C <--> Bridge4.ethg[0];
        Bridge4.ethg[1] <--> C <--> Bridge6.ethg[3];
        Bridge5.ethg[3] <--> C <--> Bridge6.ethg[1];

        Bridge5.ethg[1] <--> C <--> T0.Connection;
        Bridge5.ethg[2] <--> C <--> T1.Connection;
        Bridge6.ethg[2] <--> C <--> T2.Connection;
        Bridge4.ethg[2] <--> C <--> T3.Connection;
}

//
// Bridge 1ad testing
//
network Bridge1ADNetwork2
{
    @display("bgb=689,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        Bridge1: Ether1ADBridge {
            parameters:
                @display("p=274,39");

            gates:
                ethg[2];

        }
        Bridge2: Ether1ADBridge {
            parameters:
                @display("p=177,111");

            gates:
                ethg[4];

        }
        Bridge3: Ether1ADBridge {
            parameters:
                @display("p=392,111");

            gates:
                ethg[3];

        }
        Bridge4: Ether1ADBridge {
            parameters:
                @display("p=448,199");

            gates:
                ethg[3];

        }
        Bridge5: Ether1ADBridge {
            parameters:
                @display("p=128,199");

            gates:
                ethg[4];

        }
        Bridge6: Ether1ADBridge {
            parameters:
                @display("p=274,199");

            gates:
                ethg[4];

        }
        E0: EthernetHost {
            parameters:
                @display("p=79,271");
            gates:
                ethg;
        }
        E1: EthernetHost {
            parameters:
                @display("p=167,271");
            gates:
                ethg;
        }
        E2: EthernetHost {
            parameters:
                @display("p=281,271");
            gates:
               ethg;
        }
        E3: EthernetHost {
            parameters:
                @display("p=448,271");
            gates:
                ethg;
        }
    connections:
        Bridge1.ethg[0] <--> C <--> Bridge2.ethg[0];
        Bridge1.ethg[1] <--> C <--> Bridge3.ethg[0];
        Bridge2.ethg[1] <--> C <--> Bridge5.ethg[0];
        Bridge2.ethg[2] <--> C <--> Bridge6.ethg[0];
        Bridge2.ethg[3] <--> C <--> Bridge3.ethg[1];
        Bridge3.ethg[2] <--> C <--> Bridge4.ethg[0];
        Bridge4.ethg[1] <--> C <--> Bridge6.ethg[3];
        Bridge5.ethg[3] <--> C <--> Bridge6.ethg[1];

        Bridge5.ethg[1] <--> C <--> E0.ethg;
        Bridge5.ethg[2] <--> C <--> E1.ethg;
        Bridge6.ethg[2] <--> C <--> E2.ethg;
        Bridge4.ethg[2] <--> C <--> E3.ethg;
}
