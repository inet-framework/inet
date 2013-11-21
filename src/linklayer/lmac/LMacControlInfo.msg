class noncobject MACAddress;

cplusplus {{
    #include "MACAddress.h"
}}

class LMacControlInfo
{
    @customize(true);

    MACAddress src;  // src MAC address (can be left empty when sending)
    MACAddress dest; // dest MAC address
    int interfaceId;
}
