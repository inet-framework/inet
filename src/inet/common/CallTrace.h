//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CALLTRACE_H
#define __INET_CALLTRACE_H

#if defined(__GNUC__) && !defined(_WIN32)

#include "inet/common/INETDefs.h"

#include <regex>

namespace inet {

/**
 * This class supports generating function call traces on the standard output.
 * For example, CALL_TRACE(10, "inet::.*") will produce a function call trace
 * for a maximum depth limit of 10 levels including only INET functions.
 * Here is another example focusing on certain classes and ignoring some functions:
 * CALL_TRACE(20, "^inet::(ScenarioManager|LifecycleController|NodeStatus|NetworkInterface|InterfaceTable|Ipv4RoutingTable|Ipv4NetworkConfigurator|Ipv4NodeConfigurator|EthernetMac|EthernetMacBase)::(?!(get|is|has|compute)).*");
 * The output contains one line for entering and one line for leaving a function.
 *
 * Important: compile INET using GCC with '-finstrument-functions' into an executable.
 */
class INET_API CallTrace
{
  public:
    struct State
    {
      public:
        bool enabled = false;
        int level = -1;

        const char *name = nullptr;
        int maxLevel = -1;
        std::regex filter;
    };

  public:
    static State state;

  public:
    CallTrace(const char *name, int maxLevel, const char *filter);
    ~CallTrace();

    static std::string demangle(const char* mangledName);
};

#define CALL_TRACE(maxLevel, filter) std::string name = CallTrace::demangle(typeid(*this).name()) + "::" + __func__ + "()"; CallTrace callTrace(name.c_str(), maxLevel, filter)

}  // namespace inet

#else

#define CALL_TRACE(maxLevel, filter)

#endif

#endif
