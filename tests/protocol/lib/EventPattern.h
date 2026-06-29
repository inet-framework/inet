//
// Protocol Test Framework for INET -- Phase 1: event pattern (selector + timing).
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_EVENTPATTERN_H
#define __INET_PROTOCOLTEST_EVENTPATTERN_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "PacketEvent.h"
#include "PacketField.h"

namespace inet { class PacketFilter; }

namespace inet {
namespace protocoltest {

// Values captured from earlier matched events, keyed by name.
typedef std::map<std::string, cValue> CaptureStore;

// Context handed to a lambda predicate: the event under test plus the values
// captured by earlier steps (read with get()).
struct MatchContext {
    const PacketEvent& event;
    const CaptureStore& captures;

    cValue get(const char *name) const
    {
        auto it = captures.find(name);
        if (it == captures.end())
            throw cRuntimeError("ProtocolTest: capture '%s' is not set yet", name);
        return it->second;
    }
};

typedef std::function<bool(const MatchContext&)> MatchPredicate;
typedef std::function<cValue(const PacketEvent&)> CaptureFn;

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
    std::string selNode;                                  // on(path): subscribe/source-subtree (matched as a path prefix on the emitter)
    std::string selSource;                                // source(path): emitting-module filter (path prefix)
    std::string selSignal;                                // signal(name): registered signal name ("" = any)
    std::string selProtocol;                              // protocol(name): packet's PacketProtocolTag ("" = any)
    std::string selDispatch;                              // dispatch(name): packet's DispatchProtocolReq ("" = any)
    std::string attributeToPath;                          // attributeTo(path): description-only point of view
    bool protocolSubject = false;                         // describe the protocol module as the subject (set by sends()/receives())
    std::string selIface;                                 // "" = any interface
    bool selHasKind = false; EventKind selKind = EventKind::Other;
    bool selHasDirection = false; int selDirection = -1;  // 0=IN, 1=OUT
    bool selHasLayer = false; Layer selLayer = Layer::Unknown;
    std::string selExpr;                                  // "" = no content expression
    MatchPredicate predicate;                             // optional typed lambda predicate
    std::string description;                              // optional human phrase for the content (esp. a lambda)
    std::vector<std::pair<std::string, CaptureFn>> captures; // values to bind when this step matches
    // timing (relative to the step anchor)
    bool selHasWithin = false; simtime_t selWithin = 0;       // deadline
    bool selHasNotBefore = false; simtime_t selNotBefore = 0; // earliest

    mutable std::shared_ptr<PacketFilter> filter;         // compiled lazily from selExpr

  public:
    EventPattern& iface(const char *name) { selIface = name; return *this; }
    // --- new orthogonal selector vocabulary (pattern-language refactor) ---
    EventPattern& source(const char *path) { selSource = path; return *this; }     // emitting-module filter
    EventPattern& signal(const char *name) { selSignal = name; return *this; }     // which signal (registered name)
    EventPattern& dispatch(const char *name) { selDispatch = name; return *this; } // DispatchProtocolReq protocol
    EventPattern& packet(const char *expression) { selExpr = expression; return *this; } // PacketFilter content (value is a packet)
    EventPattern& attributeTo(const char *path) { attributeToPath = path; return *this; } // description point of view

    // Narrow to packets of a given protocol (the PacketProtocolTag name, e.g. "mobileipv6").
    EventPattern& protocol(const char *name) { selProtocol = name; return *this; }

    // Protocol-module perspective. The named protocol (see protocol()/on("node.protocol"))
    // sends a packet down to / receives a packet up from the layer below it -- written and
    // described with the protocol module as the subject, not the layer that carries it.
    // Mechanically: sends() observes the lower layer "received from upper", receives() its
    // "sent to upper", filtered to this protocol.
    EventPattern& sends(const char *matchExpr) { selHasKind = true; selKind = EventKind::ReceivedFromUpper; protocolSubject = true; selExpr = matchExpr; return *this; }
    EventPattern& receives(const char *matchExpr) { selHasKind = true; selKind = EventKind::SentToUpper; protocolSubject = true; selExpr = matchExpr; return *this; }

    EventPattern& sentToLower() { selHasKind = true; selKind = EventKind::SentToLower; return *this; }
    EventPattern& receivedFromLower() { selHasKind = true; selKind = EventKind::ReceivedFromLower; return *this; }
    EventPattern& sentToUpper() { selHasKind = true; selKind = EventKind::SentToUpper; return *this; }
    EventPattern& receivedFromUpper() { selHasKind = true; selKind = EventKind::ReceivedFromUpper; return *this; }
    EventPattern& dropped() { selHasKind = true; selKind = EventKind::Dropped; return *this; }
    EventPattern& inbound() { selHasDirection = true; selDirection = 0; return *this; }
    EventPattern& outbound() { selHasDirection = true; selDirection = 1; return *this; }
    EventPattern& layer(Layer l) { selHasLayer = true; selLayer = l; return *this; }
    EventPattern& match(const char *expression) { selExpr = expression; return *this; }
    EventPattern& match(MatchPredicate p) { predicate = std::move(p); return *this; }
    EventPattern& describe(const char *phrase) { description = phrase; return *this; }
    EventPattern& capture(const char *name, CaptureFn fn) { captures.emplace_back(name, std::move(fn)); return *this; }
    // Declarative capture: remember a "protocol.field" value (e.g. "tcp.sequenceNo").
    EventPattern& capture(const char *name, const char *fieldPath)
    {
        std::string path = fieldPath;
        captures.emplace_back(name, [path](const PacketEvent& e) { return evalPacketField(e.packet, path); });
        return *this;
    }
    EventPattern& within(double t) { selHasWithin = true; selWithin = t; return *this; }
    EventPattern& after(double t) { selHasNotBefore = true; selNotBefore = t; return *this; }
    EventPattern& notBefore(double t) { selHasNotBefore = true; selNotBefore = t; return *this; }

    // True if the selector scope (node / kind / direction / layer / interface) matches,
    // ignoring the content expression/predicate. Used by strict mode.
    bool scopeMatches(const PacketEvent& event) const;
    // True if the (non-timing) selector + content + predicate parts match.
    bool selectorMatches(const MatchContext& context) const;
    // Human-readable form for diagnostics (full English rendering is Phase 8).
    std::string str() const;

  private:
    // Evaluate the content expression, substituting {capture} placeholders.
    bool matchesExpression(const MatchContext& context) const;
};

// Entry point of the fluent selector chain.
EventPattern on(const char *nodeName);

} // namespace protocoltest
} // namespace inet

#endif
