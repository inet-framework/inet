cplusplus {{
    #include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

class noncobject MACAddress;

class SimpleLinkLayerControlInfo
{
    @customize(true);

    MACAddress src;  // can be left empty when sending
    MACAddress dest;
    int interfaceId;
}
