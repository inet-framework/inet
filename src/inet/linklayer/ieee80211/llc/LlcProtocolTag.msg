import inet.common.INETDefs;
import inet.common.TagBase;
import inet.common.Units;

cplusplus {{
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolGroup.h"
}}

namespace inet::ieee80211;

//
// LLC protocols available in IEEE 802.11
//
cplusplus {{

static ProtocolGroup llc("llc", {
    { 0, &Protocol::ieee8022 },
    { 1, &Protocol::ieee80211EtherType }
});

}}

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
    virtual const Protocol *getProtocol() const { return llc.findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) { setProtocolId(static_cast<LlcProtocolId>(protocol->getId())); }
}}

