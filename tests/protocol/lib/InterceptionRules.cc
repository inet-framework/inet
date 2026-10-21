//
// Protocol Test Framework for INET -- the ordered rules of a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "InterceptionRules.h"

namespace inet {
namespace protocoltest {

void InterceptionRules::add(const Interception& interception)
{
    Rule rule;
    rule.interception = interception;
    rules.push_back(std::move(rule));
    // Compile the expression into the rule's own filter. A PacketFilter is set once;
    // setting it again on the same object is what a shared compile step would do.
    auto& added = rules.back();
    if (!added.interception.matchExpression.empty()) {
        added.filter = std::make_shared<PacketFilter>();
        added.filter->setExpression(added.interception.matchExpression.c_str());
    }
}

InterceptionRules::Rule *InterceptionRules::select(const Packet *packet)
{
    for (auto& rule : rules) {
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
        const auto& interception = rule.interception;
        if (interception.minimumBytes > 0 && packet->getByteLength() < interception.minimumBytes)
            continue;
        rule.numSelected++;
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
