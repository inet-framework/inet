//
// MVRP messages definition
//
//


class noncobject MACAddress;
class noncobject EtherFrame;

cplusplus {{
#include "MACAddress.h"
#include "EtherFrame.h"
#include "8021Q.h"
}}


//
// Multiple VLAN Registration Protocol Data Unit
//
packet MVRPDU extends EtherFrame
{
    byteLength = 35;
    string displayString = "b=,,,#ffa5a5";
    unsigned int VIDS [];	//Registrated VLANS
    unsigned int PortIndex;  //Output port if an outgoing frame. Input port if an ingoing frame.
}

