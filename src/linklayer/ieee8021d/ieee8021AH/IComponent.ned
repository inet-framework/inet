package inet.linklayer.ieee8021d.ieee8021AH;

//******************************************************
// @brief 802.1ah B-Component cache.
// Basic conversion from IEEE 802.1ad frames to IEEE 802.1ah frames.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple IComponent
{
    
    parameters:
        @display("i=block/dispatch;bgb=429,213");
   		int defaultVID = default(-1); 		// Default BVLAN for this IComponent frames. 
   		int defaultSVID= default(-1);		// Default SVid. If outputFrame<2
   		int defaultCVID= default(-1);		// Default CVid. If outputFrame<1
   		int requestVID = default(-1);		// Requested CVid if outputFrame=1
        double interFrameTime @unit("s") = default(2s); // number of seconds it waits between MVRP messages.
        xml configIS;        // xml with itagtable info. 
        int numGates;		// number of client gates.
        int outputFrame = default (2); // 0=Ethernet 1=802.1q  2=802.1ad
        bool verbose; //Module verbosity

        gates:
        output CGatesOut[numGates]; // To the client
        input CGatesIn[numGates];	// From the client
        input IGatesIn;	//From the backbone
        output IGatesOut;	//To the backbone
}
