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

// Phase 1 has only `expect`; inject / expectNo / combinators arrive in later phases.
enum class StepType { Expect };

struct Step {
    StepType type = StepType::Expect;
    EventPattern pattern;
};

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
        steps.push_back(Step{StepType::Expect, pattern});
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
