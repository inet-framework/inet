//******************************************************
// @brief Testing message generator
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************

package inet.linklayer.ieee8021d.Tester;

//
//TesterObject. Testing message generator
//
simple TesterObject
{
    parameters:
        @display("i=block/network2");	
        xml Messages = default(xmldoc("P1.xml"));  
        bool verbose = default(true);
        bool testing = default(false);
        bool detailedRSTP = default(false);
        bool detailedMVRP = default(false);
        bool detailedEthII = default(false);  //Print the last  EthernetII received message
        bool detailedEth1Q = default(false); //Print the last Ethernet IEEE 802.1Q received message
        bool detailedEth1ad = default(false); //Print the last Ethernet IEEE 802.1ad received message
        bool detailedEth1ah = default(false); //Print the last Ethernet IEEE 802.1ah received message
     gates:
      	input In;
      	output Out;
}
