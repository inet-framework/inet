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

// Step kinds:
//  Expect    -- advance when a matching event is observed (fail on deadline)
//  Optional  -- advance on a matching event, or skip when the deadline passes
//  ExpectNo  -- fail if a matching event occurs within the window; else advance
//  Inject    -- fire a crafted packet (reactively), then advance
//  Unordered -- a group of patterns that must all match, in any order
enum class StepType { Expect, Optional, ExpectNo, Inject, Unordered };

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

struct Step {
    StepType type = StepType::Expect;
    EventPattern pattern;              // Expect / Optional / ExpectNo
    Injection injection;               // Inject
    std::vector<EventPattern> group;   // Unordered
};

// Entry point of the fluent injection chain.
Injection inject(const char *nodeName);

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

    explicit ProtocolTest(const char *name) : name(name) {}

    ProtocolTest& expect(const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::Expect, pattern, {}, {}});
        return *this;
    }

    // 0-or-1: match if it occurs within the window, otherwise skip and advance.
    ProtocolTest& optional(const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::Optional, pattern, {}, {}});
        return *this;
    }

    // Negative: fail if a matching event occurs within the window, else advance.
    ProtocolTest& expectNo(const EventPattern& pattern)
    {
        steps.push_back(Step{StepType::ExpectNo, pattern, {}, {}});
        return *this;
    }

    // All patterns must match, in any order, before advancing. The group window is
    // the longest within() among its patterns.
    ProtocolTest& unordered(std::vector<EventPattern> patterns)
    {
        steps.push_back(Step{StepType::Unordered, {}, {}, std::move(patterns)});
        return *this;
    }

    ProtocolTest& inject(const Injection& injection)
    {
        steps.push_back(Step{StepType::Inject, {}, injection, {}});
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
};

struct INET_API ProtocolTestRegistrar {
    ProtocolTestRegistrar(const char *name, ProtocolTestBuilderFn builder) { ProtocolTestRegistry::add(name, builder); }
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

} // namespace protocoltest
} // namespace inet

#endif
