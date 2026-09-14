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
    // A value that carries a unit keeps it. intValue() and doubleValue() throw on such a
    // value -- "Attempt to use the value '24B' as a dimensionless number" -- and the
    // expression engine reads the quantity form, so 24B substitutes and compares correctly.
    const char *unit = value.getUnit();
    bool hasUnit = unit != nullptr && *unit != '\0';
    switch (value.getType()) {
        case cValue::INT: return hasUnit ? value.str() : std::to_string(value.intValue());
        case cValue::BOOL: return value.boolValue() ? "true" : "false";
        case cValue::DOUBLE: {
            if (hasUnit)
                return value.str();
            std::ostringstream os; os << value.doubleValue(); return os.str();
        }
        default: return value.str();
    }
}

EventPattern on(const char *path)
{
    // A module path (the subscribe/source subtree): "MN[0]", "MN[0].wlan[0]",
    // "MN[0].ipv6.ipv6". Matched as a component-aligned prefix of the emitting module.
    EventPattern pattern;
    pattern.fltNode = path;
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
    if (!pathMatches(fltNode, event.sourcePath))    // on(path): emitter subtree
        return false;
    if (!pathMatches(fltSource, event.sourcePath))  // source(path): emitter filter
        return false;
    if (!fltSignal.empty() && fltSignal != event.signalName)
        return false;
    if (!fltProtocol.empty() && fltProtocol != event.protocolName)
        return false;
    if (!fltDispatch.empty() && fltDispatch != event.dispatchName)
        return false;
    if (fltHasDirection && event.direction != fltDirection)
        return false;
    if (!fltIface.empty() && fltIface != event.interfaceName)
        return false;
    return true;
}

// Evaluates one content expression against the event's packet, with the same capture
// substitution and the same "absent protocol is a non-match" rule the filter side uses.
// An assertion runs once per step, so it compiles fresh rather than caching.
static bool evaluateAssertionExpression(const std::string& text, const MatchContext& context)
{
    const Packet *packet = context.event.packet;
    if (packet == nullptr)
        return false;
    std::string expression = text;
    for (auto& capture : context.captures) {
        std::string placeholder = "{" + capture.first + "}";
        if (expression.find(placeholder) == std::string::npos)
            continue;
        std::string value = formatCaptureValue(capture.second);
        size_t pos;
        while ((pos = expression.find(placeholder)) != std::string::npos)
            expression.replace(pos, placeholder.size(), value);
    }
    if (expression.find('{') != std::string::npos)
        return false;
    try {
        PacketFilter filter;
        filter.setExpression(expression.c_str());
        return filter.matches(packet);
    }
    catch (const std::exception&) {
        return false;
    }
}

static std::string describeValue(double v)
{
    std::ostringstream os;
    os << v;
    return os.str();
}

bool EventPattern::assertionsHold(const MatchContext& context, std::string& reason) const
{
    const PacketEvent& event = context.event;
    for (auto& assertion : assertions) {
        switch (assertion.kind) {
            case Assertion::Packet:
                if (!evaluateAssertionExpression(assertion.expr, context)) {
                    reason = "the packet does not satisfy '" + assertion.expr + "'";
                    return false;
                }
                break;
            case Assertion::NotPacket:
                if (evaluateAssertionExpression(assertion.expr, context)) {
                    reason = "the packet satisfies '" + assertion.expr + "', which is forbidden";
                    return false;
                }
                break;
            case Assertion::Event:
                try {
                    if (assertion.predicate && !assertion.predicate(context)) {
                        reason = "the predicate does not hold";
                        return false;
                    }
                }
                catch (const std::exception& e) {
                    reason = std::string("the predicate raised: ") + e.what();
                    return false;
                }
                break;
            case Assertion::Value:
            case Assertion::NotValue:
            case Assertion::ValueAtLeast:
            case Assertion::ValueAtMost: {
                if (!event.hasValue) {
                    reason = "the event carries no scalar value";
                    return false;
                }
                const char *relation = nullptr;
                if (assertion.kind == Assertion::Value && event.value != assertion.value)
                    relation = "equal to";
                else if (assertion.kind == Assertion::NotValue && event.value == assertion.value)
                    relation = "different from";
                else if (assertion.kind == Assertion::ValueAtLeast && event.value < assertion.value)
                    relation = "at least";
                else if (assertion.kind == Assertion::ValueAtMost && event.value > assertion.value)
                    relation = "at most";
                if (relation) {
                    reason = "the value is " + describeValue(event.value) + ", and it must be "
                             + relation + " " + describeValue(assertion.value);
                    return false;
                }
                break;
            }
        }
    }
    return true;
}

bool EventPattern::selectorMatches(const MatchContext& context) const
{
    const PacketEvent& event = context.event;
    if (!scopeMatches(event))
        return false;
    if (fltHasMin && (!event.hasValue || event.value < fltMin))       // scalar signal lower bound
        return false;
    if (fltHasMax && (!event.hasValue || event.value > fltMax))       // scalar signal upper bound
        return false;
    if (fltHasValue && (!event.hasValue || event.value != fltValue))  // scalar signal value
        return false;
    // A packet-field expression needs a packet. A predicate does not: it receives the whole
    // context, so it can judge a scalar signal's value, the time, or a capture. Refusing it
    // here made every rule about a scalar unwritable, and silently: the step simply never
    // matched, so a guard over a scalar held over nothing and the check could not fail.
    // The QUIC pass reported this from the other side, when a running total could not be
    // accumulated on the signal side.
    if (event.packet == nullptr && !fltExpr.empty())
        return false;
    if (!fltExpr.empty() && !matchesExpression(context))
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
    if (fltExpr.find('{') == std::string::npos) {
        if (!filter) {
            filter = std::make_shared<PacketFilter>();
            filter->setExpression(fltExpr.c_str()); // throws on a malformed expression
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
    std::string expression = fltExpr;
    for (auto& capture : context.captures) {
        std::string placeholder = "{" + capture.first + "}";
        // Only convert a capture this expression names. Converting every stored capture
        // made one step fail on a capture that belonged to another step.
        if (expression.find(placeholder) == std::string::npos)
            continue;
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
    os << "on " << (fltNode.empty() ? "*" : fltNode);
    if (!fltSource.empty()) os << " source=" << fltSource;
    if (!fltSignal.empty()) os << " signal=" << fltSignal;
    if (!fltProtocol.empty()) os << " protocol=" << fltProtocol;
    if (!fltDispatch.empty()) os << " dispatch=" << fltDispatch;
    if (fltHasDirection) os << " dir=" << (fltDirection == 0 ? "IN" : "OUT");
    if (!fltIface.empty()) os << " iface=" << fltIface;
    if (!fltExpr.empty()) os << " expr='" << fltExpr << "'";
    if (predicate) os << " predicate";
    if (fltHasValue) os << " value=" << fltValue;
    if (fltHasMin) os << " value>=" << fltMin;
    if (fltHasMax) os << " value<=" << fltMax;
    if (fltOccurrence == 1) os << " first";
    else if (fltOccurrence > 1) os << " nth=" << fltOccurrence;
    if (fltHasNotBefore) os << " notBefore=" << fltNotBefore;
    if (fltHasWithin) os << " within=" << fltWithin;
    // The assertion half is rendered apart from the filter half, because the two say
    // different things: everything before the arrow picked the event, everything after it
    // had to hold on the event that was picked.
    if (!assertions.empty()) {
        os << " -> asserts";
        const char *separator = " ";
        for (auto& assertion : assertions) {
            os << separator;
            separator = ", ";
            switch (assertion.kind) {
                case Assertion::Packet: os << "packet '" << assertion.expr << "'"; break;
                case Assertion::NotPacket: os << "not packet '" << assertion.expr << "'"; break;
                case Assertion::Event: os << "a predicate on the event"; break;
                case Assertion::Value: os << "value == " << assertion.value; break;
                case Assertion::NotValue: os << "value != " << assertion.value; break;
                case Assertion::ValueAtLeast: os << "value >= " << assertion.value; break;
                case Assertion::ValueAtMost: os << "value <= " << assertion.value; break;
            }
        }
    }
    return os.str();
}

} // namespace protocoltest
} // namespace inet
