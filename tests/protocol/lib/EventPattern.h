//
// Protocol Test Framework for INET -- Phase 1: event pattern (selector + timing).
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_EVENTPATTERN_H
#define __INET_PROTOCOLTEST_EVENTPATTERN_H

#include <memory>
#include <string>

#include "PacketEvent.h"

namespace inet { class PacketFilter; }

namespace inet {
namespace protocoltest {

//
// A fluent selector + timing clause, as produced by on("node").sentToLower()...
// In Phase 1 it backs `expect` steps. The selector part (node/iface/kind/direction/
// layer + a PacketFilter content expression) decides which events match; the timing
// part (within/notBefore, relative to the step's anchor) is interpreted by the engine.
//
class INET_API EventPattern
{
  public:
    // selector
    std::string selNode;                                  // "" = any node
    std::string selIface;                                 // "" = any interface
    bool selHasKind = false; EventKind selKind = EventKind::Other;
    bool selHasDirection = false; int selDirection = -1;  // 0=IN, 1=OUT
    bool selHasLayer = false; Layer selLayer = Layer::Unknown;
    std::string selExpr;                                  // "" = no content predicate
    // timing (relative to the step anchor)
    bool selHasWithin = false; simtime_t selWithin = 0;       // deadline
    bool selHasNotBefore = false; simtime_t selNotBefore = 0; // earliest

    mutable std::shared_ptr<PacketFilter> filter;         // compiled lazily from selExpr

  public:
    EventPattern& iface(const char *name) { selIface = name; return *this; }
    EventPattern& sentToLower() { selHasKind = true; selKind = EventKind::SentToLower; return *this; }
    EventPattern& receivedFromLower() { selHasKind = true; selKind = EventKind::ReceivedFromLower; return *this; }
    EventPattern& sentToUpper() { selHasKind = true; selKind = EventKind::SentToUpper; return *this; }
    EventPattern& receivedFromUpper() { selHasKind = true; selKind = EventKind::ReceivedFromUpper; return *this; }
    EventPattern& dropped() { selHasKind = true; selKind = EventKind::Dropped; return *this; }
    EventPattern& inbound() { selHasDirection = true; selDirection = 0; return *this; }
    EventPattern& outbound() { selHasDirection = true; selDirection = 1; return *this; }
    EventPattern& layer(Layer l) { selHasLayer = true; selLayer = l; return *this; }
    EventPattern& match(const char *expression) { selExpr = expression; return *this; }
    EventPattern& within(double t) { selHasWithin = true; selWithin = t; return *this; }
    EventPattern& after(double t) { selHasNotBefore = true; selNotBefore = t; return *this; }
    EventPattern& notBefore(double t) { selHasNotBefore = true; selNotBefore = t; return *this; }

    // True if the (non-timing) selector part matches this event.
    bool selectorMatches(const PacketEvent& event) const;
    // Human-readable form for diagnostics (full English rendering is Phase 8).
    std::string str() const;
};

// Entry point of the fluent selector chain.
EventPattern on(const char *nodeName);

} // namespace protocoltest
} // namespace inet

#endif
