//
// Protocol Test Framework for INET -- the ordered rules of a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "InterceptionRules.h"

namespace inet {
namespace protocoltest {

int InterceptionRules::add(const Interception& interception, const CaptureStore *captures)
{
    if (interception.action == "duplicate") {
        if (interception.onlyCopies)
            throw cRuntimeError("tap '%s': a rule cannot duplicate the copies of another rule (copiesOnly() with duplicate())",
                                interception.tapName.c_str());
        if (interception.numCopies < 1 || interception.spacing < SIMTIME_ZERO)
            throw cRuntimeError("tap '%s': duplicate() needs at least one copy and a spacing that is not negative",
                                interception.tapName.c_str());
    }
    Rule rule;
    rule.interception = interception;
    rule.captures = captures;
    rule.active = !interception.startsAtStep;
    rule.startTime = simTime();
    rules.push_back(std::move(rule));
    // Compile the expression into the rule's own filter. A PacketFilter is set once;
    // setting it again on the same object is what a shared compile step would do. An
    // expression that names captures compiles fresh for each frame instead.
    auto& added = rules.back();
    const auto& expression = added.interception.matchExpression;
    if (!expression.empty() && expression.find('{') == std::string::npos) {
        added.filter = std::make_shared<PacketFilter>();
        added.filter->setExpression(expression.c_str());
    }
    return (int)rules.size() - 1;
}

void InterceptionRules::start(int index)
{
    auto& rule = rules.at(index);
    rule.active = true;
    rule.startTime = simTime();
}

InterceptionRules::Rule *InterceptionRules::select(const Frame& frame)
{
    const Packet *packet = frame.packet;
    for (auto& rule : rules) {
        const auto& interception = rule.interception;
        // A rule acts only between its start and its end, and it counts only what it sees
        // there.
        if (!rule.active || (interception.hasWithin && simTime() >= rule.startTime + interception.withinTime))
            continue;
        // A copy goes through the rules again, but not through the rule that made it, and a
        // copiesOnly() rule sees nothing else.
        if (&rule == frame.copyOf || (interception.onlyCopies && frame.copyOf == nullptr))
            continue;
        if (!interception.fromNode.empty() && interception.fromNode != frame.fromNode)
            continue;
        if (!interception.toNode.empty() && interception.toNode != frame.toNode)
            continue;
        if (rule.filter != nullptr) {
            bool matched = false;
            try {
                matched = rule.filter->matches(packet);
            }
            catch (const std::exception&) {
                // An expression that doesn't apply to this frame (e.g. a tcp.* test on an
                // ARP frame) is simply a non-match, never an error.
                matched = false;
            }
            if (!matched)
                continue;
        }
        else if (!interception.matchExpression.empty()) {
            // The expression names captures: it compiles fresh with the values of now, and a
            // capture that does not exist yet makes it a non-match.
            static const CaptureStore noCaptures;
            if (!matchesWithCaptures(interception.matchExpression, rule.captures != nullptr ? *rule.captures : noCaptures, packet))
                continue;
        }
        if (interception.minimumBytes > 0 && packet->getByteLength() < interception.minimumBytes)
            continue;
        // A rule counts frames, not receptions.
        if (frame.frameId < 0 || frame.frameId != rule.lastFrameId) {
            rule.numSelected++;
            rule.lastFrameId = frame.frameId;
        }
        bool wanted = interception.fromOccurrence > 0 ? rule.numSelected >= interception.fromOccurrence
                                                      : (interception.occurrence == 0 || rule.numSelected == interception.occurrence);
        if (wanted) {
            numSelected++;
            return &rule;
        }
    }
    return nullptr;
}

} // namespace protocoltest
} // namespace inet
