package inet.linklayer.ieee8021d.ieee8021AH;

//******************************************************
// @brief 802.1ah Filter
// 802.1ah Filtering 
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple PortFilt1ah
{
    
    parameters:
		int cost = default(1);	// RSTP port cost
		int priority = default(1);	// RSTP port priority
    gates:
        output GatesOut[2]; // 1 (inner side) allways tagged. 0 depends on tagged variable
        input GatesIn[2];	// 1 (inner side) allways tagged. 0 depends on tagged variable
}
