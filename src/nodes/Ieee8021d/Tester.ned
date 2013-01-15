//
// Tester node for 802.1ah, 802.1ad, 802.1q, Ethernet MVRP and RSTP
// 
//

package inet.nodes.Ieee8021d;
import inet.linklayer.ethernet.EtherMAC;
import inet.linklayer.ieee8021d.Tester.TesterObject;

//
//	Tester node.
//
module Tester
{
    parameters:
        @node();
        @display("i=device/port;bgb=643,442");
		
    gates:
        inout Connection;  
       
    submodules:
        testerObject: TesterObject {
            parameters:
                @display("p=171,148");
            gates:
                Out;
                In;
        }
        mac: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=171,53,row;q=queue");
        }

    connections:
		mac.upperLayerIn <-- testerObject.Out;
		mac.upperLayerOut --> testerObject.In;
		mac.phys <-->Connection;
}
