class noncobject MACAddress;

cplusplus {{
    #include "MACAddress.h"
}}

packet BMacFrame
{
    MACAddress destAddr;   // destination mac address
    MACAddress srcAddr;    // source mac address
}