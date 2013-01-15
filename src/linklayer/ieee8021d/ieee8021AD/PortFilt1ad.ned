package inet.linklayer.ieee8021d.ieee8021AD;


//******************************************************
// @brief 802.1Q Tagging and Filtering.
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple PortFilt1ad
{
    
    parameters:
       	bool tagged=default(true);
       	int defaultVID = default(-1);   //Default S-VID
       	xml confCVIDs = default(xmldoc("confCVIDs.xml")); //Registered C-Vids
        bool verbose=default(false); //Module verbosity       
        double interFrameTime @unit("s") = default(2s); // number of seconds it waits between MVRP messages.
        int cost=default(1);
        int priority=default(5);
    gates:
        output GatesOut[2]; // 1 (Inner side) allways tagged. 0 depends on tagged variable
        input GatesIn[2];	// 1 (Inner side) allways tagged. 0 depends on tagged variable
}
