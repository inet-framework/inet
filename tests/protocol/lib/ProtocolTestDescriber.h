//
// Protocol Test Framework for INET -- English description generator (plan §13).
//
// Deterministic AST -> text rendering of a ProtocolTest program. Pure (no runtime),
// so it doubles as living documentation, a CI golden file, and Qtenv hover-text.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PROTOCOLTESTDESCRIBER_H
#define __INET_PROTOCOLTEST_PROTOCOLTESTDESCRIBER_H

#include <string>

namespace inet {
namespace protocoltest {

class ProtocolTest;

// Render a program as a numbered English description.
std::string describe(const ProtocolTest& test);

} // namespace protocoltest
} // namespace inet

#endif
