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
// A fluent selector + timing clause, as produced by on("path").signal(...).packet(...).
// The selector part (module path / source / signal / protocol / dispatch / iface + a
// PacketFilter content expression, or a scalar value via is()) decides which events match;
// the timing part (within/notBefore, relative to the step's anchor) is interpreted by the engine.
//
class INET_API EventPattern
{
  public:
    // selector
    std::string fltNode;                                  // on(path): subscribe/source-subtree (matched as a path prefix on the emitter)
    std::string fltSource;                                // source(path): emitting-module filter (path prefix)
    std::string fltSignal;                                // signal(name): registered signal name ("" = any)
    std::string fltProtocol;                              // protocol(name): packet's PacketProtocolTag ("" = any)
    std::string fltDispatch;                              // dispatch(name): packet's DispatchProtocolReq ("" = any)
    bool fltHasValue = false; double fltValue = 0;          // is(v): scalar signal value (e.g. an FSM state index)
    bool fltHasMin = false; double fltMin = 0;              // isAtLeast(v): lower bound on a scalar signal
    bool fltHasMax = false; double fltMax = 0;              // isAtMost(v): upper bound on a scalar signal
    std::string attributeToPath;                          // attributeTo(path): description-only point of view
    std::string fltIface;                                 // "" = any interface
    bool fltHasDirection = false; int fltDirection = -1;  // 0=IN, 1=OUT
    std::string fltExpr;                                  // "" = no content expression
    MatchPredicate predicate;                             // optional typed lambda predicate
    std::string description;                              // optional human phrase for the content (esp. a lambda)
    std::vector<std::pair<std::string, CaptureFn>> captures; // values to bind when this step matches
    // timing (relative to the step anchor)
    bool fltHasWithin = false; simtime_t fltWithin = 0;       // deadline
    bool fltHasNotBefore = false; simtime_t fltNotBefore = 0; // earliest

    mutable std::shared_ptr<PacketFilter> filter;         // compiled lazily from fltExpr

    // --- assertion: what must hold on the event the filter picked ---
    //
    // A filter and an assertion are two different things, and a word says which it is. A
    // filter picks the event: when nothing matches, the step misses its deadline. An
    // assertion must hold on the picked event: when it does not, the step fails at once and
    // the engine never looks for a later event. That difference is the whole point of the
    // split, because a value written as a filter turns a wrong first value into a silent
    // search for a right later one.
    struct Assertion {
        enum Kind { Packet, NotPacket, Event, Value, NotValue, ValueAtLeast, ValueAtMost } kind = Packet;
        std::string expr;                                 // Expr / NotExpr
        MatchPredicate predicate;                         // That
        double value = 0;                                 // Equal / NotEqual / AtLeast / AtMost
    };
    std::vector<Assertion> assertions;

    // Which event of the filtered stream the assertions speak about. 0 means "the engine
    // decides": the first filtered event when there are assertions, and today's behaviour
    // (any matching event) when there are none.
    int fltOccurrence = 0;
    mutable int fltHits = 0;                              // filtered events seen so far

  public:
    EventPattern& iface(const char *name) { fltIface = name; return *this; }
    // --- new orthogonal selector vocabulary (pattern-language refactor) ---
    EventPattern& source(const char *path) { fltSource = path; return *this; }     // emitting-module filter
    EventPattern& signal(const char *name) { fltSignal = name; return *this; }     // which signal (registered name)
    EventPattern& dispatch(const char *name) { fltDispatch = name; return *this; } // DispatchProtocolReq protocol
    // A standard usually states a bound rather than a value: "at most 4 segments", "no less
    // than one second". These express such a bound on a scalar signal.
    EventPattern& attributeTo(const char *path) { attributeToPath = path; return *this; } // description point of view

    // Narrow to packets of a given protocol (the PacketProtocolTag name, e.g. "mobileipv6").
    EventPattern& protocol(const char *name) { fltProtocol = name; return *this; }

    EventPattern& inbound() { fltHasDirection = true; fltDirection = 0; return *this; }
    EventPattern& outbound() { fltHasDirection = true; fltDirection = 1; return *this; }
    // --- filter words: they pick the event ---
    EventPattern& filterPacket(const char *expression) { fltExpr = expression; return *this; }
    EventPattern& filterEvent(MatchPredicate p) { predicate = std::move(p); return *this; }
    EventPattern& filterValue(double v) { fltHasValue = true; fltValue = v; return *this; }
    EventPattern& filterValueAtLeast(double v) { fltHasMin = true; fltMin = v; return *this; }
    EventPattern& filterValueAtMost(double v) { fltHasMax = true; fltMax = v; return *this; }
    EventPattern& filterValueBetween(double lo, double hi) { return filterValueAtLeast(lo).filterValueAtMost(hi); }

    // --- position words: they pick which filtered event, and compare nothing ---
    EventPattern& nth(int k) { fltOccurrence = k; return *this; }
    EventPattern& first() { return nth(1); }

    // --- assertion words: they must hold on the picked event ---
    EventPattern& assertPacket(const char *e) { assertions.push_back({Assertion::Packet, e, nullptr, 0}); return *this; }
    EventPattern& assertNotPacket(const char *e) { assertions.push_back({Assertion::NotPacket, e, nullptr, 0}); return *this; }
    EventPattern& assertEvent(MatchPredicate p) { assertions.push_back({Assertion::Event, "", std::move(p), 0}); return *this; }
    EventPattern& assertValue(double v) { assertions.push_back({Assertion::Value, "", nullptr, v}); return *this; }
    EventPattern& assertNotValue(double v) { assertions.push_back({Assertion::NotValue, "", nullptr, v}); return *this; }
    EventPattern& assertValueAtLeast(double v) { assertions.push_back({Assertion::ValueAtLeast, "", nullptr, v}); return *this; }
    EventPattern& assertValueAtMost(double v) { assertions.push_back({Assertion::ValueAtMost, "", nullptr, v}); return *this; }
    EventPattern& assertValueBetween(double lo, double hi) { return assertValueAtLeast(lo).assertValueAtMost(hi); }
    // There is deliberately no assertNotThat: a lambda negates itself, so it would be
    // exactly assertEvent of the negation. assertNotPacket is not redundant in the same way,
    // because it differs from assertPacket of a negated expression when the chunk is absent.

    // True when every assertion holds on this event. On a failure, reason says which one.
    bool assertionsHold(const MatchContext& context, std::string& reason) const;

    EventPattern& describe(const char *phrase) { description = phrase; return *this; }
    EventPattern& capture(const char *name, CaptureFn fn) { captures.emplace_back(name, std::move(fn)); return *this; }
    // Declarative capture: remember a "protocol.field" value (e.g. "tcp.sequenceNo").
    EventPattern& capture(const char *name, const char *fieldPath)
    {
        std::string path = fieldPath;
        captures.emplace_back(name, [path](const PacketEvent& e) { return evalPacketField(e.packet, path); });
        return *this;
    }
    EventPattern& within(double t) { fltHasWithin = true; fltWithin = t; return *this; }
    EventPattern& after(double t) { fltHasNotBefore = true; fltNotBefore = t; return *this; }
    EventPattern& notBefore(double t) { fltHasNotBefore = true; fltNotBefore = t; return *this; }

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
