package inet.examples.ieee802_1d.VlanEtherApp;


import inet.nodes.Ieee8021d.Ether1QBridge;
import inet.nodes.Ieee8021d.EthernetHost;
import ned.DatarateChannel;


//
// Bridge 1Q testing
//
network Bridge1QNetwork2
{
    @display("bgb=582,368");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        Bridge1: Ether1QBridge {
            parameters:
                @display("p=274,39");

            gates:
                ethg[2];

        }
        Bridge2: Ether1QBridge {
            parameters:
                @display("p=177,111");

            gates:
                ethg[4];

        }
        Bridge3: Ether1QBridge {
            parameters:
                @display("p=392,111");

            gates:
                ethg[3];

        }
        Bridge4: Ether1QBridge {
            parameters:
                @display("p=448,199");

            gates:
                ethg[4];

        }
        Bridge5: Ether1QBridge {
            parameters:
                @display("p=128,199");

            gates:
                ethg[4];

        }
        Bridge6: Ether1QBridge {
            parameters:
                @display("p=274,199");

            gates:
                ethg[5];

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
                @display("p=244,271");
            gates:
                ethg;
        }
        E3: EthernetHost {
            parameters:
                @display("p=418,271");
            gates:
                ethg;
        }
        E4: EthernetHost {
            parameters:
                @display("p=313,271");
            gates:
                ethg;
        }
        E5: EthernetHost {
            parameters:
                @display("p=487,271");
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

        Bridge4.ethg[3] <--> C <--> E5.ethg;
        Bridge6.ethg[4] <--> C <--> E4.ethg;
}

