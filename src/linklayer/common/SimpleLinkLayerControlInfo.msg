class noncobject MACAddress;

cplusplus {{
    #include "MACAddress.h"
}}

class SimpleLinkLayerControlInfo
{
    MACAddress src;  // can be left empty when sending
    MACAddress dest;
    int interfaceId;
}
