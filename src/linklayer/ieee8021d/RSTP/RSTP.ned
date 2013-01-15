package inet.linklayer.ieee8021d.RSTP;
//******************************************************
// @brief Implements the Rapid Spanning Tree Protocol for IEEE 802 6-byte MAC addresses.
// Sends the hello message and configures the Port State of each Bridge Port, selecting some Bridge Ports to forward frames
// and others to discard frames, to provide a fully (spanning) and simply (tree) connected active
// topology.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple RSTP
{
    parameters:
        int BridgePriority = default(128);  				//priority + MAC determines the RSTP id
        double helloTime @unit("s") = default(2s); 			// number of seconds RSTP waits between hello messages
        double forwardDelay @unit("s") = default(6s); 	// Upgrade delay.
        double migrateTime @unit ("s")= default(30s);		//Time the bridge waits for a better root info. After todesignated time it swiches all ports which are not bk or alternate to designated.
        double TCWhileTime @unit("s") = default(6s); 		//A port will send TC=true during this time
        @display("i=block/network2");	
        int MaxAge = default(20); 							//Max. hop count. IEEE 802.1D typical range 6-40
        bool verbose=default(false);						//sets module verbosity
        bool testing=default(false);						//Saves testing data
        bool autoEdge=default(true);  						//Automatic edge setting. If false, Edge ports will have to wait to get designated.
        xml UpTimeEvents=default(xmldoc("UpTimeEvents.xml"));
    gates:
      	inout RSTPPort @labels(BPDU);						//To admacrelay
}
