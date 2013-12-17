//******************************************************
// @brief BEB. IEEE 802.1ah Backbone Edge Bridge.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/


package inet.nodes.Ieee8021d;
import inet.linklayer.ieee8021d.RSTP.BComponentRSTP;
import inet.linklayer.ieee8021d.ieee8021Q.PortFilt;
import inet.linklayer.ethernet.EtherMAC;
import inet.linklayer.ieee8021d.RSTP.RSTP;
import inet.linklayer.ieee8021d.MVRP.MVRP;

//
// Model of Backbone Edge Bridge (BEB)
// It can also be used as a Backbone bridge
//
// This model uses the RSTP (rapid spanning tree) and the MVRP (multiple vlan registration protocol) module
//
module RSTPSwitch
{
    parameters:
        @node();
        @display("i=device/router;bgb=643,476;bgl=2");
    gates:
        inout ethgB[];  //To the backbone
    submodules:
        bcomponent: BComponentRSTP {
            parameters:
                @display("p=183,178");
                sizeEthgB = sizeof(ethgB);
            gates:
                BportsOut[sizeof(ethgB)];
                BportsIn[sizeof(ethgB)];
                RSTPPort;
        } 												//Relay unit. Present in all backbone nodes.

        macB[sizeof(ethgB)]: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=183,330,row");
        }												// IComponent data managing.

        rstp: RSTP {
            @display("p=47,109");
            gates:
                RSTPPort;
        }												// RSTP protocol managing.

    connections:

        for i=0..sizeof(ethgB)-1 {
            ethgB[i] <--> macB[i].phys;
            bcomponent.BportsOut[i] --> macB[i].upperLayerIn;
            bcomponent.BportsIn[i] <-- macB[i].upperLayerOut;
        }

        rstp.RSTPPort <--> bcomponent.RSTPPort;
}
