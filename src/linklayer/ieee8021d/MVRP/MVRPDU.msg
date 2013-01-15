//
// MVRP messages definition
//
//



class noncobject MACAddress;
class noncobject EtherFrame;

cplusplus {{
#include "MACAddress.h"
#include "EtherFrame_m.h"
#include "8021Q.h"
}}






//
// Multiple VLAN Registration Protocol Data Unit
//
packet MVRPDU extends EtherFrame
{
    string displayString = "b=,,,#ffa5a5";
    @customize(true);			//Allows BPDU_m.cc customization. Generator will create BPDU_Base
    unsigned int VIDS [];	//Registrated VLANS
    unsigned int PortIndex;  //Output port if an outgoing frame. Input port if an ingoing frame.
}


