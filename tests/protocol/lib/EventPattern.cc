//
// Protocol Test Framework for INET -- Phase 1: event pattern (selector + timing).
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "EventPattern.h"

#include <sstream>

#include "inet/common/packet/PacketFilter.h"

namespace inet {
namespace protocoltest {

EventPattern on(const char *nodeName)
{
    EventPattern pattern;
    pattern.selNode = nodeName;
    return pattern;
}

bool EventPattern::selectorMatches(const PacketEvent& event) const
{
    if (!selNode.empty() && (event.node == nullptr || selNode != event.node->getFullName()))
        return false;
    if (selHasKind && event.kind != selKind)
        return false;
    if (selHasDirection && event.direction != selDirection)
        return false;
    if (selHasLayer && event.layer != selLayer)
        return false;
    if (!selIface.empty() && selIface != event.interfaceName)
        return false;
    if (!selExpr.empty()) {
        if (!filter) {
            filter = std::make_shared<PacketFilter>();
            filter->setExpression(selExpr.c_str()); // throws on a malformed expression
        }
        if (event.packet == nullptr)
            return false;
        // A content expression referencing a protocol absent from this packet (e.g.
        // `udp.*` on an ARP frame) throws during evaluation; that is simply a non-match.
        // Constrain selectors by layer/kind so expressions mostly see relevant packets.
        try {
            if (!filter->matches(event.packet))
                return false;
        }
        catch (const std::exception&) {
            return false;
        }
    }
    return true;
}

std::string EventPattern::str() const
{
    std::ostringstream os;
    os << "on " << (selNode.empty() ? "*" : selNode);
    if (selHasKind) os << " " << getEventKindName(selKind);
    if (selHasDirection) os << " dir=" << (selDirection == 0 ? "IN" : "OUT");
    if (selHasLayer) os << " layer=" << getLayerName(selLayer);
    if (!selIface.empty()) os << " iface=" << selIface;
    if (!selExpr.empty()) os << " match='" << selExpr << "'";
    if (selHasNotBefore) os << " notBefore=" << selNotBefore;
    if (selHasWithin) os << " within=" << selWithin;
    return os.str();
}

} // namespace protocoltest
} // namespace inet
