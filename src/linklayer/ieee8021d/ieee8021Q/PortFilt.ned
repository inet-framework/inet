package inet.linklayer.ieee8021d.ieee8021Q;

//******************************************************
// @brief 802.1Q Tagging and filtering skills
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple PortFilt
{
    
    parameters:
       	bool tagged=default(true);
       	int defaultVID = default(-1);
        bool verbose=default(false); //Module verbosity
        double interFrameTime @unit("s") = default(2s); // number of seconds it waits between MVRP messages.
        int cost=default(1);
        int priority=default(5);
    gates:
        output GatesOut[2]; // 1 (inner port) allways tagged. 0 depends on tagged variable
        input GatesIn[2];	// 1 (inner port) allways tagged. 0 depends on tagged variable
}
