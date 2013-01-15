package inet.examples.ieee802_1d.Generales;

import inet.nodes.Ieee8021d.BEB;
import inet.nodes.Ieee8021d.Ether1ADBridge;
import inet.nodes.Ieee8021d.Ether1QBridge;
import inet.nodes.Ieee8021d.EthernetHost;
import ned.DatarateChannel;



network CompleteProviderNetwork1
{
    @display("bgb=1200,609;bgi=background/Leyenda,s;bgs=1,km");
    types:
        channel C extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
		///PROVIDER BACKBONE
        BEB1: BEB {
            parameters:
                @display("p=425,58");

            gates:
                ethgB[3];
        }
        BEB2: BEB {
            parameters:
                @display("p=546,58");

            gates:
                ethgB[4];

        }
        BEB3: BEB {
            parameters:
                @display("p=261,122");

            gates:
                ethgB[2];
                ethgC[1];
        }
        BEB4: BEB {
            parameters:
                @display("p=404,122");

            gates:
                ethgB[3];
        }

        BEB5: BEB {
            parameters:
                @display("p=591,122");

            gates:
                ethgB[2];
                ethgC[1];
        }
        BEB6: BEB {
            parameters:
                @display("p=851,122");

            gates:
                ethgB[2];
                ethgC[1];
        }


        /// 802.1AD BRIDGES
        BridgeAD1: Ether1ADBridge {
            parameters:
                @display("p=261,317");

            gates:
                ethg[3];
        }
        BridgeAD2: Ether1ADBridge {
            parameters:
                @display("p=591,259");

            gates:
                ethg[3];
        }
        BridgeAD3: Ether1ADBridge {
            parameters:
                @display("p=510,317");

            gates:
                ethg[3];
        }
        BridgeAD4: Ether1ADBridge {
            parameters:
                @display("p=674,317");

            gates:
                ethg[3];
        }
        /// Client 802.1Q Bridges
        BridgeQ1: Ether1QBridge {
            parameters:
                @display("p=202,440");

            gates:
                ethg[3];
        }
        BridgeQ2: Ether1QBridge {
            parameters:
                @display("p=319,440");

            gates:
                ethg[3];
        }
        BridgeQ3: Ether1QBridge {
            parameters:
                @display("p=510,440");

            gates:
                ethg[3];
        }
        BridgeQ4: Ether1QBridge {
            parameters:
                @display("p=674,440");

            gates:
                ethg[3];
        }

        BridgeQ5: Ether1QBridge {
            parameters:
                @display("p=851,394");

            gates:
                ethg[3];
        }
        BridgeQ6: Ether1QBridge {
            parameters:
                @display("p=783,440");

            gates:
                ethg[3];
        }
        BridgeQ7: Ether1QBridge {
            parameters:
                @display("p=937,440");

            gates:
                ethg[3];
        }


        ///HOSTS

        hostA: EthernetHost {
            parameters:
                @display("p=126,551");
        }
        hostB: EthernetHost {
            parameters:
                @display("p=202,551");
        }
        hostC: EthernetHost {
            parameters:
                @display("p=319,551");
        }
        hostD: EthernetHost {
            parameters:
                @display("p=404,551");
        }
        hostE: EthernetHost {
            parameters:
                @display("p=471,551");
        }
        hostF: EthernetHost {
            parameters:
                @display("p=547,551");
        }
        hostG: EthernetHost {
            parameters:
                @display("p=637,551");
        }
        hostH: EthernetHost {
            parameters:
                @display("p=713,551");
        }

        hostI: EthernetHost {
            parameters:
                @display("p=783,551");
        }

        hostJ: EthernetHost {
            parameters:
                @display("p=937,491");
        }


    connections:


        BridgeQ1.ethg[1] <--> C <--> hostA.ethg;
        BridgeQ1.ethg[2] <--> C <--> hostB.ethg;

        BridgeQ2.ethg[1] <--> C <--> hostC.ethg;
        BridgeQ2.ethg[2] <--> C <--> hostD.ethg;

        BridgeQ3.ethg[1] <--> C <--> hostE.ethg;
        BridgeQ3.ethg[2] <--> C <--> hostF.ethg;

        BridgeQ4.ethg[1] <--> C <--> hostG.ethg;
        BridgeQ4.ethg[2] <--> C <--> hostH.ethg;

        BridgeQ5.ethg[1] <--> C <--> BridgeQ6.ethg[0];
        BridgeQ5.ethg[2] <--> C <--> BridgeQ7.ethg[0];

        BridgeQ6.ethg[1] <--> C <--> hostI.ethg;
        BridgeQ6.ethg[2] <--> C <--> BridgeQ7.ethg[1];

        BridgeQ7.ethg[2] <--> C <--> hostJ.ethg;

        BridgeAD1.ethg[1] <--> C <--> BridgeQ1.ethg[0];
        BridgeAD1.ethg[2] <--> C <--> BridgeQ2.ethg[0];

        BridgeAD3.ethg[1] <--> C <--> BridgeQ3.ethg[0];
        BridgeAD4.ethg[2] <--> C <--> BridgeQ4.ethg[0];
        BridgeAD2.ethg[1] <--> C <--> BridgeAD3.ethg[0];
        BridgeAD2.ethg[2] <--> C <--> BridgeAD4.ethg[0];
        BridgeAD3.ethg[2] <--> C <--> BridgeAD4.ethg[1];

        BEB3.ethgC[0] <--> C <--> BridgeAD1.ethg[0];
        BEB5.ethgC[0] <--> C <--> BridgeAD2.ethg[0];

        BEB1.ethgB[0] <--> C <--> BEB3.ethgB[0];
        BEB1.ethgB[1] <--> C <--> BEB4.ethgB[0];
        BEB1.ethgB[2] <--> C <--> BEB2.ethgB[0];

        BEB2.ethgB[1] <--> C <--> BEB4.ethgB[2];
        BEB2.ethgB[2] <--> C <--> BEB5.ethgB[0];
        BEB2.ethgB[3] <--> C <--> BEB6.ethgB[0];

        BEB3.ethgB[1] <--> C <--> BEB4.ethgB[1];

        BEB5.ethgB[1] <--> C <--> BEB6.ethgB[1];

        BEB6.ethgC[0] <--> C <--> BridgeQ5.ethg[0];
}
