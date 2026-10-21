//
// Protocol Test Framework for INET -- the ordered rules of a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_INTERCEPTIONRULES_H
#define __INET_PROTOCOLTEST_INTERCEPTIONRULES_H

#include <deque>
#include <memory>

#include "inet/common/packet/PacketFilter.h"
#include "ProtocolTest.h"

namespace inet {
namespace protocoltest {

//
// The rules of one tap. A tap holds a list of rules, not one rule: four standards passes
// worked around a single-rule tap by putting two taps in series, and one of them had to
// write down the arithmetic of which occurrence the second tap sees. A frame is offered to
// the rules in order and the first that matches applies; a frame that matches none passes.
// Each rule counts its own occurrences, so two rules can name the second and the third
// frame of the same kind without either counting the other's.
//
class INET_API InterceptionRules
{
  public:
    struct Rule {
        Interception interception;
        // By pointer, and never by value: a PacketFilter does not survive a copy, and a
        // Rule is copied when it goes into the container. The pointer is made in place.
        std::shared_ptr<PacketFilter> filter;
        long numSelected = 0;          // frames this rule has selected, for its occurrence
    };

  protected:
    std::deque<Rule> rules;
    long numSelected = 0;              // frames that any rule selected

  public:
    void add(const Interception& interception);
    void clear() { rules.clear(); }

    // The first rule that matches the frame and is at its targeted occurrence, else null.
    Rule *select(const Packet *packet);

    long getNumSelected() const { return numSelected; }
};

} // namespace protocoltest
} // namespace inet

#endif
