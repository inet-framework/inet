
import inet.linklayer.common.MACAddress;

namespace inet;

class SimpleLinkLayerControlInfo extends cObject
{
    @customize(true);

    MACAddress src;  // can be left empty when sending
    MACAddress dest;
    int interfaceId;
}
