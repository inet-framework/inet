package inet.linklayer.ieee8021d.MVRP;
//******************************************************
// @brief Multiple VLAN Registration Protocol for IEE 802 6-byte MAC Address
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/
simple MVRP
{
    parameters:
        double interFrameTime @unit("s") = default(2s); // number of seconds MVRP waits between messages
        double agingTime @unit("s") = default(6s); // Maximum idle time for a MVRP entry
       @display("i=block/classifier");
        bool verbose= default(false); // Sets verbose mode if true
        bool testing= default(false); // Saves testing data
    gates:
      	inout MVRPPort @labels(MVRPDU);  //To Admacrelay
}
