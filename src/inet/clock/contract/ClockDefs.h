//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CLOCKDEFS_H
#define __INET_CLOCKDEFS_H

#include <iomanip>
#include <iostream>
#include <sstream>

#include "inet/common/INETDefs.h"

namespace inet {

// TODO: move this debugging stuff into a separate file
struct NullStream {
    template<typename T>
    NullStream& operator<<(const T&) { return *this; }
    NullStream& operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
};

extern NullStream nullStream;

#ifndef CLOCK_LOG_IMPLEMENTATION
  #define CLOCK_COUT nullStream
#else
  #define CLOCK_COUT if (!clockCoutEnabled) ; else std::cout << std::setprecision(24) << std::string(clockCoutIndentLevel * 3, ' ')
#endif

#ifndef NDEBUG
#define CLOCK_CHECK_IMPLEMENTATION
#endif

// TODO: move the operator into the middle of the macro
#ifdef CLOCK_CHECK_IMPLEMENTATION
#define ASSERTCMP(cmp, o1, o2) { \
    ClockCoutDisabledBlock b; auto _o1 = (o1); auto _o2 = (o2); \
    if (!(_o1 cmp _o2)) { \
        std::ostringstream oss; \
        oss << "ASSERT: Condition '" << #o1 << " " << #cmp << " " << #o2 << "' as '" << _o1 << " " << #cmp << " " << _o2 << "' does not hold"; \
        omnetpp::cRuntimeError error("in function '%s()' at %s:%d", __FUNCTION__, __FILE__, __LINE__); \
        error.prependMessage(oss.str().c_str()); \
        throw error; \
    } \
}
#else
#define ASSERTCMP(cmp, o1, o2) { (void)(o1); (void)(o2); }
#endif

extern bool clockCoutEnabled;
extern int clockCoutIndentLevel;

class ClockCoutIndent {
public:
    ClockCoutIndent() { ++clockCoutIndentLevel; }
    ~ClockCoutIndent() { --clockCoutIndentLevel; }
};

class ClockCoutDisabledBlock {
public:
    ClockCoutDisabledBlock() { clockCoutEnabled = false; }
    ~ClockCoutDisabledBlock() { clockCoutEnabled = true; }
};

} // namespace inet

#endif

