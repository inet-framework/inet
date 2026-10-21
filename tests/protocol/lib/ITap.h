//
// Protocol Test Framework for INET -- the contract of a tap.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_ITAP_H
#define __INET_PROTOCOLTEST_ITAP_H

#include "ProtocolTest.h"

namespace inet {
namespace protocoltest {

//
// A tap is a module that applies the interception rules of a test program to the frames
// it sees. The ProtocolTester installs the rules of the program's tap(...) clauses through
// this contract, so it does not need to know which kind of tap a clause names.
//
class INET_API ITap
{
  public:
    virtual ~ITap() {}

    // Appends a rule, and returns its handle on this tap. The rules of one tap apply in the
    // order they were added. The rule reads the values of its {name} placeholders from
    // `captures` each time it examines a frame. A rule of meanwhile(...) waits until
    // startInterception() names its handle.
    virtual int addInterception(const Interception& interception, const CaptureStore *captures) = 0;

    // Starts a rule that waits for its step. The rule counts frames from now on.
    virtual void startInterception(int handle) = 0;
};

} // namespace protocoltest
} // namespace inet

#endif
