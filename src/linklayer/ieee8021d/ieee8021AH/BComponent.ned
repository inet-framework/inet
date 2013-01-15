package inet.linklayer.ieee8021d.ieee8021AH;
//******************************************************
// @brief 802.1ah B-Component.
// Second layer. 802.1ah switching
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
module BComponent
{
    parameters:
        @display("i=block/broadcast;bgb=425,266");
        int ICompnumber;
        int sizeEthgB;

    gates:
        output BportsOut[];  // To the backbone and the IComponents. That is ethgC+ethgB
        input BportsIn[];	// From the backbone and the IComponents. That is ethgC+ethgB
        inout RSTPPort;	// To the RSTP module
        inout MVRPPort;	// To the MVRP module

    submodules:
        admacrelay: Admacrelay {
            parameters:
                @display("p=148,50");
            gates:
                GatesIn[sizeof(BportsIn)];
                GatesOut[sizeof(BportsOut)];
                RSTPGate;
                MVRPGate;
        }										// Relay unit
        cache: Cache1AH {
            parameters:
                @display("p=276,35");
        }

        PortFilt[sizeEthgB+ICompnumber]: PortFilt1ah {
            parameters:
                @display("p=148,152,row");
            gates:
                GatesIn[];
                GatesOut[];

        }

    connections:
        for i=0..sizeof(BportsOut)-1 {
            BportsOut[i] <-- PortFilt[i].GatesOut[0];
            PortFilt[i].GatesIn[1] <-- admacrelay.GatesOut[i];
            BportsIn[i] --> PortFilt[i].GatesIn[0];
            PortFilt[i].GatesOut[1]--> admacrelay.GatesIn[i];
        }
        admacrelay.RSTPGate <--> RSTPPort;
        admacrelay.MVRPGate <--> MVRPPort;
}
