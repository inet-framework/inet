//
// Protocol Test Framework for INET -- Phase 1: program model + registry.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PROTOCOLTEST_H
#define __INET_PROTOCOLTEST_PROTOCOLTEST_H

#include <map>
#include <string>
#include <vector>

#include "EventPattern.h"

namespace inet {
namespace protocoltest {

// Step kinds. The cardinality kinds count how many times a pattern matches; they
// model a count range [min, max] (regex-quantifier style):
//  Once         -- exactly 1: advance when a matching event is observed (fail on deadline)
//  AtMostOnce   -- 0..1: advance on a matching event, or skip when the deadline passes
//  Never        -- 0: fail if a matching event occurs within the window; else advance
//  ExactlyTimes -- exactly n: advance when the nth matching event is observed
//  Count        -- a greedy, window-bounded range [cardMin, cardMax] (cardMax < 0 =
//                  unbounded): consume every match in the window, then require the
//                  total to fall in range; a match beyond cardMax fails immediately.
//                  Backs oneOrMoreTimes/anyNumberOfTimes/atLeastTimes/atMostTimes/betweenTimes.
//  Inject       -- fire a crafted packet (reactively), then advance
//  Unordered    -- a group of patterns that must all match, in any order
//  AnyOf        -- a group of patterns; the first to match wins
//  Delivery     -- a "from" send correlated to a "to" receive of the same packet (treeId)
enum class StepType { Once, AtMostOnce, Never, ExactlyTimes, Count, Inject, Unordered, AnyOf, Delivery };

//
// A packet injection. When the engine reaches an inject step it resolves the target
// (node -> moduleSubPath -> gateName) and pushPacket()s a packet built by `builder`
// (which may read captures from earlier steps -- the reactive `use()`). The builder
// owns construction, so any chunk/tag combination is possible -- "inject arbitrary
// packets". Timing: `.at(t)` absolute, or `.after(d)` relative to the previous step's
// match (reactive); default is immediately upon reaching the step.
//
class INET_API Injection
{
  public:
    std::string nodeName;
    std::string moduleSubPath;   // relative module path under the node, e.g. "eth[0]"
    std::string gateName;        // sink gate to push to, e.g. "upperLayerOut"
    bool hasAtTime = false; simtime_t atTime = 0;
    bool hasAfter = false; simtime_t afterDelay = 0;
    std::string description;     // optional human phrase for the injected packet
    std::function<Packet *(const CaptureStore&)> builder;

    Injection& into(const char *module, const char *gate) { moduleSubPath = module; gateName = gate; return *this; }
    Injection& at(double t) { hasAtTime = true; atTime = t; return *this; }
    Injection& after(double d) { hasAfter = true; afterDelay = d; return *this; }
    Injection& describe(const char *phrase) { description = phrase; return *this; }
    Injection& packet(std::function<Packet *(const CaptureStore&)> fn) { builder = std::move(fn); return *this; }
};

//
// A man-in-the-middle interception rule, applied by a named PacketTap spliced on a link.
// The tap selects frames by a PacketFilter expression (over the dissected frame), an
// optional minimum size, and an occurrence index, and applies an action:
//   drop  -- discard the frame (force a retransmission)
//   delay -- forward it after a hold time
//   mutate-- run a C++ mutator on the (inner) frame, then forward it
// The ProtocolTester installs these on the tap module at startup (see intercept(...)).
//
class INET_API Interception
{
  public:
    std::string tapName;          // PacketTap module name (a sibling of the tester)
    std::string matchExpression;  // PacketFilter expression over the dissected frame
    long minimumBytes = 0;        // also require the (inner) frame to be at least this big
    int occurrence = 0;           // act on the Nth selected frame (1-based); 0 = every
    std::string action = "drop";  // "drop" | "delay" | "mutate"
    simtime_t delayTime = 0;
    std::function<void(Packet *)> mutator; // for action == "mutate"
    std::string description;      // optional human phrase

    Interception& match(const char *expr) { matchExpression = expr; return *this; }
    Interception& minBytes(long n) { minimumBytes = n; return *this; }
    Interception& nth(int k) { occurrence = k; return *this; }
    Interception& drop() { action = "drop"; return *this; }
    Interception& delay(double t) { action = "delay"; delayTime = t; return *this; }
    Interception& mutate(std::function<void(Packet *)> fn) { action = "mutate"; mutator = std::move(fn); return *this; }
    Interception& describe(const char *phrase) { description = phrase; return *this; }
};

struct Step {
    StepType type = StepType::Once;
    EventPattern pattern;              // Once / AtMostOnce / Never / ExactlyTimes / Count / Delivery "from"
    Injection injection;               // Inject
    std::vector<EventPattern> group;   // Unordered / AnyOf
    EventPattern pattern2;             // Delivery "to" (holds the delivery window in its within)
    int count = 0;                     // ExactlyTimes
    int cardMin = 0, cardMax = 0;      // Count: required occurrences [min, max]; max < 0 = unbounded
};

// Entry point of the fluent injection chain.
Injection inject(const char *nodeName);

// Entry point of the fluent interception chain (names the PacketTap to drive).
Interception intercept(const char *tapName);

//
// A protocol test program: an ordered list of steps with a name. Built with the
// fluent API, e.g.:
//
//   ProtocolTest("udp")
//       .expect(on("host1").sentToLower().match("udp.destPort == 5000").within(0.2))
//       .expect(on("host2").receivedFromLower().match("udp.destPort == 5000").within(0.1));
//
class INET_API ProtocolTest
{
  public:
    std::string name;
    std::vector<Step> steps;
    std::vector<Interception> interceptions;  // standing MITM rules, installed on taps at startup
    bool strictMode = false;   // closed-world: an in-scope but non-matching event fails an Expect

    explicit ProtocolTest(const char *name) : name(name) {}

    // Closed-world matching: a packet matching an Expect step's selector scope
    // (node/kind/layer/iface) but not its content predicate is a failure.
    ProtocolTest& strict() { strictMode = true; return *this; }

    // Exactly 1: advance when one matching event is observed (fail on deadline).
    ProtocolTest& once(const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::Once, pattern, {}, {}});
        return *this;
    }

    // 0 or 1: match if it occurs within the window, otherwise skip and advance.
    ProtocolTest& atMostOnce(const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::AtMostOnce, pattern, {}, {}});
        return *this;
    }

    // 0: fail if a matching event occurs within the window, else advance.
    ProtocolTest& never(const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::Never, pattern, {}, {}});
        return *this;
    }

    // Exactly n: advance when the nth matching event is observed (fail on deadline).
    ProtocolTest& exactlyTimes(int n, const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::ExactlyTimes, pattern, {}, {}, {}, n});
        return *this;
    }

    // Greedy, window-bounded cardinalities. Each consumes every matching event within
    // the pattern's within() window, then requires the total in the given range; a
    // match beyond the upper bound fails immediately. The count argument comes first.
    ProtocolTest& oneOrMoreTimes(const EventPattern& pattern)             { return addCount(pattern, 1, -1); }  // 1..*
    ProtocolTest& anyNumberOfTimes(const EventPattern& pattern)          { return addCount(pattern, 0, -1); }  // 0..*
    ProtocolTest& atLeastTimes(int n, const EventPattern& pattern)       { return addCount(pattern, n, -1); }  // n..*
    ProtocolTest& atMostTimes(int n, const EventPattern& pattern)        { return addCount(pattern, 0, n); }   // 0..n
    ProtocolTest& betweenTimes(int a, int b, const EventPattern& pattern) { return addCount(pattern, a, b); }  // a..b

    // All patterns must match, in any order, before advancing. The group window is
    // the longest within() among its patterns.
    ProtocolTest& unordered(std::vector<EventPattern> patterns)
    {
        steps.push_back(Step{StepType::Unordered, {}, {}, std::move(patterns)});
        return *this;
    }

    // The first of the given patterns to match wins; advance on it (fail on deadline).
    ProtocolTest& anyOf(std::vector<EventPattern> patterns)
    {
        steps.push_back(Step{StepType::AnyOf, {}, {}, std::move(patterns)});
        return *this;
    }

    // A packet matching `from` (a send) is then received matching `to` -- the same
    // packet (correlated by treeId) -- within `window`.
    ProtocolTest& delivery(const EventPattern& from, const EventPattern& to, double window)
    {
        EventPattern toWindowed = to;
        toWindowed.within(window);
        steps.push_back(Step{StepType::Delivery, from, {}, {}, toWindowed, 0});
        return *this;
    }

    ProtocolTest& inject(const Injection& injection)
    {
        steps.push_back(Step{StepType::Inject, {}, injection, {}});
        return *this;
    }

    // A standing man-in-the-middle rule installed on the named PacketTap at startup. Not
    // an ordered step -- it is armed for the whole run.
    ProtocolTest& intercept(const Interception& interception)
    {
        interceptions.push_back(interception);
        return *this;
    }

  private:
    ProtocolTest& addCount(const EventPattern& pattern, int cardMin, int cardMax)
    {
        Step step{StepType::Count, pattern, {}, {}};
        step.cardMin = cardMin;
        step.cardMax = cardMax;
        steps.push_back(step);
        return *this;
    }
};

// A registered program is produced by a builder function (so construction is lazy
// and happens inside the simulation, after INET is initialized).
typedef ProtocolTest (*ProtocolTestBuilderFn)();

class INET_API ProtocolTestRegistry
{
  public:
    static std::map<std::string, ProtocolTestBuilderFn>& all();
    static void add(const char *name, ProtocolTestBuilderFn builder);
    static bool has(const char *name);
    static ProtocolTest build(const char *name);

    // The single unnamed "default" program (Define_ProtocolTestProgram). The ProtocolTester
    // runs it when no testName is set, so a self-contained test needs no name/selection.
    static void setDefault(ProtocolTestBuilderFn builder);
    static bool hasDefault();
    static ProtocolTest buildDefault();
};

struct INET_API ProtocolTestRegistrar {
    ProtocolTestRegistrar(const char *name, ProtocolTestBuilderFn builder) { ProtocolTestRegistry::add(name, builder); }
    explicit ProtocolTestRegistrar(ProtocolTestBuilderFn builder) { ProtocolTestRegistry::setDefault(builder); }
};

//
// Registers a protocol test program. Usage:
//
//   Define_ProtocolTest(udp_basic) {
//       return ProtocolTest("udp_basic").expect(...);
//   }
//
// The stringized identifier is the test name selected via the ProtocolTester's
// `testName` parameter.
//
#define Define_ProtocolTest(id)                                                                      \
    static inet::protocoltest::ProtocolTest protocolTestBuilder_##id();                              \
    static inet::protocoltest::ProtocolTestRegistrar protocolTestRegistrar_##id(#id, &protocolTestBuilder_##id); \
    static inet::protocoltest::ProtocolTest protocolTestBuilder_##id()

//
// Defines the single program for this build (no name, no selection): the ProtocolTester
// runs it automatically when its `testName` is empty. Use this for a self-contained test
// where exactly one program is compiled in. Usage:
//
//   Define_ProtocolTestProgram() {
//       return ProtocolTest("my_test").once(...);
//   }
//
#define Define_ProtocolTestProgram()                                                                 \
    static inet::protocoltest::ProtocolTest protocolTestDefaultBuilder_();                           \
    static inet::protocoltest::ProtocolTestRegistrar protocolTestDefaultRegistrar_(&protocolTestDefaultBuilder_); \
    static inet::protocoltest::ProtocolTest protocolTestDefaultBuilder_()

} // namespace protocoltest
} // namespace inet

#endif
