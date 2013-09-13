//******************************************************
// @brief BEB. IEEE 802.1ah Backbone Edge Bridge.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/


package inet.nodes.Ieee8021d;
import inet.base.NotificationBoard;
import inet.networklayer.common.InterfaceTable;
import inet.linklayer.IWiredNic;
import inet.linklayer.ethernet.switch.MACAddressTable;
import inet.linklayer.ieee8021d.RSTP.RSTP;
import inet.linklayer.ieee8021d.RSTP.RelayRSTP;
import inet.linklayer.ieee8021d.RSTP.PortFiltRSTP;

module RSTPSwitch
{
    parameters:
        @node();
        @display("i=device/router;bgb=386,346;bgl=2");
        eth[*].encapType = "EtherEncapDummy";
    gates:
        inout ethg[];
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=50,92;is=s");
        }
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=50,150;is=s");
        }
        relay: RelayRSTP {
            parameters:
                @display("p=199,50");
            gates:
                GatesIn[sizeof(ethg)];
                GatesOut[sizeof(ethg)];
                RSTPGate;
        }
        cache: MACAddressTable {
            parameters:
                @display("p=316,50");
        }

        PortFilt[sizeof(ethg)]: PortFiltRSTP {
            parameters:
                @display("p=199,150,row");
            gates:
                GatesIn[];
                GatesOut[];

        }
        
        eth[sizeof(ethg)]: <default("EthernetInterface")> like IWiredNic {
            parameters:
                mac.promiscuous = true;
                @display("p=199,281,row;q=txQueue");
        }
        
        rstp: RSTP {
            @display("p=127,50");
            gates:
                RSTPPort;
        }												// RSTP protocol managing.

    connections:
        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            PortFilt[i].GatesOut[0] --> eth[i].upperLayerIn;
            PortFilt[i].GatesIn[0] <-- eth[i].upperLayerOut;
            PortFilt[i].GatesIn[1] <-- relay.GatesOut[i];
            PortFilt[i].GatesOut[1] --> relay.GatesIn[i];
        }

        rstp.RSTPPort <--> relay.RSTPGate;
}
