import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

cplusplus {{
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
}}

namespace inet::ieee80211;

enum LlcProtocolId
{
    LPD = 0; // IEEE 802.2 LLC header (with SNAP)
    EPD = 1; // plain EtherType header
}

//
// Marks the packet's LLC protocol
// NOTE: No packet field indicates the LLC protocol, it depends on the used band!
//
class LlcProtocolTag extends TagBase
{
    int protocolId @enum(LlcProtocolId);
}

cplusplus(LlcProtocolTag) {{
    virtual const Protocol *getProtocol() const { return ProtocolGroup::getIeee80211LlcProtocolGroup()->findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) { setProtocolId(ProtocolGroup::getIeee80211LlcProtocolGroup()->getProtocolNumber(protocol)); }
}}
