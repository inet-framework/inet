//******************************************************
// @brief BEB. IEEE 802.1ah Backbone Edge Bridge.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/


package inet.nodes.Ieee8021d;
import inet.linklayer.ieee8021d.ieee8021AH.IComponent;
import inet.linklayer.ieee8021d.ieee8021AH.ITagTable;
import inet.linklayer.ieee8021d.ieee8021AH.BComponent;
import inet.linklayer.ieee8021d.ieee8021AH.PortFilt1ah;
import inet.linklayer.ethernet.EtherMAC;
import inet.linklayer.ieee8021d.RSTP.RSTP;
import inet.linklayer.ieee8021d.MVRP.MVRP;

//
// Model of Backbone Edge Bridge (BEB)
// It can also be used as a Backbone bridge
//
// This model uses the RSTP (rapid spanning tree) and the MVRP (multiple vlan registration protocol) module
//
module BEB
{
    parameters:
        @node();
        @display("i=device/router;bgb=643,476;bgl=2");
        int ICompnumber = default(0);  /// Number of IComponents present in the BEB. ICompnumber<sizeof(ethgC[])  MAX 6. ICompnumber>6 needs BEB.ned modification.
    gates:
        inout ethgB[];  //To the backbone
        inout ethgC[];  //To the client network
    submodules:
        bcomponent: BComponent {
            parameters:
                @display("p=183,178");
                ICompnumber = ICompnumber;
                sizeEthgB = sizeof(ethgB);
            gates:
                BportsOut[sizeof(ethgB)+ICompnumber];
                BportsIn[sizeof(ethgB)+ICompnumber];
                RSTPPort;
                MVRPPort;
        } 												//Relay unit. Present in all backbone nodes.

        icomponent[ICompnumber]: IComponent {
            parameters:
                @display("p=183,303,row");
            gates:
                CGatesOut[];
                CGatesIn[];
                IGatesIn;
                IGatesOut;
        }												//802.1ad-802.1ah conversion.

        itagtable: ITagTable {
            parameters:
                @display("p=47,264");
        }
        macB[sizeof(ethgB)]: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=183,91,row");
        }												// IComponent data managing.
        macC[sizeof(ethgC)]: EtherMAC {
            parameters:
                promiscuous = true;
                queueModule = "";
                @display("p=183,399,row");
        }

        rstp: RSTP {
            @display("p=47,109");
            gates:
                RSTPPort;
        }												// RSTP protocol managing.
        mvrp: MVRP {
            @display("p=47,178");
            gates:
                MVRPPort;
        }												// MVRP protocol managing.

    connections:
        for i=0..sizeof(ethgC)-1 {
            ethgC[i] <--> macC[i].phys;
        }
        // IComponent - BComponent
        for i=0..ICompnumber-1 {
            icomponent[i].IGatesIn <-- bcomponent.BportsOut[i+sizeof(ethgB)]; 
            icomponent[i].IGatesOut --> bcomponent.BportsIn[i+sizeof(ethgB)];
        }
        //Connections through the IComponents, in case they exist.
        if ICompnumber>0 {
            macC[i].upperLayerIn <-- icomponent[0].CGatesOut[i] for i=0..icomponent[0].numGates-1;
            macC[i].upperLayerOut --> icomponent[0].CGatesIn[i] for i=0..icomponent[0].numGates-1;
        }
        if ICompnumber>1 {
            macC[i+icomponent[0].numGates].upperLayerIn <-- icomponent[1].CGatesOut[i] for i=0..icomponent[1].numGates-1;
            macC[i+icomponent[0].numGates].upperLayerOut --> icomponent[1].CGatesIn[i] for i=0..icomponent[1].numGates-1;
        }
        if ICompnumber>2 {
            macC[i+icomponent[0].numGates+icomponent[1].numGates].upperLayerIn <-- icomponent[2].CGatesOut[i] for i=0..icomponent[2].numGates-1;
            macC[i+icomponent[0].numGates+icomponent[1].numGates].upperLayerOut --> icomponent[2].CGatesIn[i] for i=0..icomponent[2].numGates-1;
        }
        if ICompnumber>3 {
            macC[i+icomponent[0].numGates+icomponent[1].numGates+icomponent[2].numGates].upperLayerIn <-- icomponent[3].CGatesOut[i] for i=0..icomponent[3].numgates-1;
            macC[i+icomponent[0].numGates+icomponent[1].numGates+icomponent[2].numGates].upperLayerOut --> icomponent[3].CGatesIn[i] for i=0..icomponent[3].numgates-1;
        }
        if ICompnumber>4 {
            macC[i+icomponent[0].numGates+icomponent[1].numGates+icomponent[2].numGates+icomponent[3].numGates].upperLayerIn <-- icomponent[4].CGatesOut[i] for i=0..icomponent[4].numgates-1;
            macC[i+icomponent[0].numGates+icomponent[1].numGates+icomponent[2].numGates+icomponent[3].numGates].upperLayerOut --> icomponent[4].CGatesIn[i] for i=0..icomponent[4].numgates-1;
        }
        if ICompnumber>5 {
            macC[i+icomponent[0].numGates+icomponent[1].numGates+icomponent[2].numGates+icomponent[3].numGates+icomponent[4].numGates].upperLayerIn <-- icomponent[5].CGatesOut[i] for i=0..icomponent[5].numgates-1;
            macC[i+icomponent[0].numGates+icomponent[1].numGates+icomponent[2].numGates+icomponent[3].numGates+icomponent[4].numGates].upperLayerOut --> icomponent[5].CGatesIn[i] for i=0..icomponent[5].numgates-1;
        }

        ///Add a wider range of IComponent copying the next if clause and modifying it to match this formula. Where n+1 is the max number of IComponents pressent in one BEB.
        //
        //   if ICompnumber>n {
        //   macC[i+icomponent[0].numGates+ ... +icomponent[n-1].numGates].upperLayerIn <-- icomponent[n].CGatesOut[i] for i=0..icomponent[n].numgates-1;
        //   macC[i+icomponent[0].numGates+ ... +icomponent[n-1].numGates].upperLayerOut --> icomponent[n].CGatesIn[i] for i=0..icomponent[5].numgates-1;
        //   }
        //
        //


        for i=0..sizeof(ethgB)-1 {
            ethgB[i] <--> macB[i].phys;
            bcomponent.BportsOut[i] --> macB[i].upperLayerIn;
            bcomponent.BportsIn[i] <-- macB[i].upperLayerOut;
        }

        rstp.RSTPPort <--> bcomponent.RSTPPort;
        mvrp.MVRPPort <--> bcomponent.MVRPPort;
}
