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

EventPattern on(const char *path)
{
    // A module path (the subscribe/source subtree): "MN[0]", "MN[0].wlan[0]",
    // "MN[0].ipv6.ipv6". Matched as a component-aligned prefix of the emitting module.
    EventPattern pattern;
    pattern.selNode = path;
    return pattern;
}

// Component-aligned path prefix: pattern "MN[0]" matches "MN[0]" and "MN[0].wlan[0].mac"
// but not "MN[0]extra"; an empty pattern matches anything.
static bool pathMatches(const std::string& pattern, const std::string& path)
{
    if (pattern.empty())
        return true;
    if (path == pattern)
        return true;
    return path.size() > pattern.size() &&
           path.compare(0, pattern.size(), pattern) == 0 &&
           path[pattern.size()] == '.';
}

bool EventPattern::scopeMatches(const PacketEvent& event) const
{
    if (!pathMatches(selNode, event.sourcePath))    // on(path): emitter subtree
        return false;
    if (!pathMatches(selSource, event.sourcePath))  // source(path): emitter filter
        return false;
    if (!selSignal.empty() && selSignal != event.signalName)
        return false;
    if (!selProtocol.empty() && selProtocol != event.protocolName)
        return false;
    if (!selDispatch.empty() && selDispatch != event.dispatchName)
        return false;
    if (selHasDirection && event.direction != selDirection)
        return false;
    if (!selIface.empty() && selIface != event.interfaceName)
        return false;
    return true;
}

bool EventPattern::selectorMatches(const MatchContext& context) const
{
    const PacketEvent& event = context.event;
    if (!scopeMatches(event))
        return false;
    if (selHasValue && (!event.hasValue || event.value != selValue))  // scalar signal value
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
    if (!selSource.empty()) os << " source=" << selSource;
    if (!selSignal.empty()) os << " signal=" << selSignal;
    if (!selProtocol.empty()) os << " protocol=" << selProtocol;
    if (!selDispatch.empty()) os << " dispatch=" << selDispatch;
    if (selHasDirection) os << " dir=" << (selDirection == 0 ? "IN" : "OUT");
    if (!selIface.empty()) os << " iface=" << selIface;
    if (!selExpr.empty()) os << " match='" << selExpr << "'";
    if (selHasNotBefore) os << " notBefore=" << selNotBefore;
    if (selHasWithin) os << " within=" << selWithin;
    return os.str();
}

} // namespace protocoltest
} // namespace inet
