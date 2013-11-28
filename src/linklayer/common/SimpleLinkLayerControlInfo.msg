class noncobject MACAddress;

cplusplus {{
    #include "MACAddress.h"
}}

class SimpleLinkLayerControlInfo
{
    @customize(true);

    MACAddress src;  // can be left empty when sending
    MACAddress dest;
    int interfaceId;
}
