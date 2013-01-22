//******************************************************
// @brief BPDU message definition
// @author Juan Luis Garrote Molinero
// @version 1.0
// @date Feb 2011
//******************************************************/


class noncobject MACAddress;
class noncobject EtherFrame;

cplusplus {{
#include "MACAddress.h"
#include "EtherFrame.h"
}}


//
// BPDU
//
packet BPDUieee8021D extends EtherFrame
{
    byteLength = 35;
    string displayString = "b=,,,#a5a5ff";
    int rootPriority;			//Proposed root priority. rootPriority+rootMAC define the proposed root id
    MACAddress rootMAC;			//Proposed root MAC
    int cost;					//Cost from root
    int SrcPriority;  			//id priority. Priority + srcMAC define the bridge id
    int PortPriority;			//Priority of the tx port.
    int PortNumber;				//Sender Bridge output port.
    int age;					//Message age  
    bool TC;					//Topology change flag
    bool Ack;					//Acknowledgement flag
}

