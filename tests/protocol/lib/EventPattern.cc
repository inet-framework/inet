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

static std::string formatCaptureValue(const cValue& value)
{
    switch (value.getType()) {
        case cValue::INT: return std::to_string(value.intValue());
        case cValue::BOOL: return value.boolValue() ? "true" : "false";
        case cValue::DOUBLE: { std::ostringstream os; os << value.doubleValue(); return os.str(); }
        default: return value.str();
    }
}

EventPattern on(const char *nodeName)
{
    EventPattern pattern;
    pattern.selNode = nodeName;
    return pattern;
}

bool EventPattern::selectorMatches(const MatchContext& context) const
{
    const PacketEvent& event = context.event;
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
    if (event.packet == nullptr && (!selExpr.empty() || predicate))
        return false;
    if (!selExpr.empty() && !matchesExpression(context))
        return false;
    if (predicate) {
        // Same robustness: a predicate that peeks a chunk absent from this packet
        // throws; treat that as a non-match rather than aborting the run.
        try {
            if (!predicate(context))
                return false;
        }
        catch (const std::exception&) {
            return false;
        }
    }
    return true;
}

bool EventPattern::matchesExpression(const MatchContext& context) const
{
    const Packet *packet = context.event.packet;

    // Static expression (no {capture} placeholders): compile once and cache.
    if (selExpr.find('{') == std::string::npos) {
        if (!filter) {
            filter = std::make_shared<PacketFilter>();
            filter->setExpression(selExpr.c_str()); // throws on a malformed expression
        }
        // A content expression referencing a protocol absent from this packet (e.g.
        // `udp.*` on an ARP frame) throws during evaluation; that is simply a non-match.
        // Constrain selectors by layer/kind so expressions mostly see relevant packets.
        try {
            return filter->matches(packet);
        }
        catch (const std::exception&) {
            return false;
        }
    }

    // Dynamic expression: substitute {name} with captured values, then compile fresh.
    std::string expression = selExpr;
    for (auto& capture : context.captures) {
        std::string placeholder = "{" + capture.first + "}";
        std::string value = formatCaptureValue(capture.second);
        size_t pos;
        while ((pos = expression.find(placeholder)) != std::string::npos)
            expression.replace(pos, placeholder.size(), value);
    }
    if (expression.find('{') != std::string::npos)
        return false; // an unresolved {capture} -- not satisfiable yet
    try {
        PacketFilter dynamicFilter;
        dynamicFilter.setExpression(expression.c_str());
        return dynamicFilter.matches(packet);
    }
    catch (const std::exception&) {
        return false;
    }
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
