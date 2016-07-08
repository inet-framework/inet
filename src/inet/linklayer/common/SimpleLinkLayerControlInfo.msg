cplusplus {{
    #include "inet/linklayer/common/MACAddress.h"
}}

namespace inet;

class noncobject MACAddress;

class SimpleLinkLayerControlInfo
{
    @customize(true);

    int protocol = -1;
    int socketId = -1;
}
