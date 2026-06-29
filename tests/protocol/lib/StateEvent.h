//
// Protocol Test Framework for INET -- Phase 9: normalised scalar-signal event.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_STATEEVENT_H
#define __INET_PROTOCOLTEST_STATEEVENT_H

#include <string>

#include "inet/common/INETDefs.h"

namespace inet {
namespace protocoltest {

//
// A normalised observation of a non-packet scalar signal -- e.g. an FSM's state index
// (INET's Fsm::setStateChangedSignal emits the state on every transition), a counter, or
// a transmit-opportunity ID. The second observation channel beside PacketEvent: it lets a
// program assert module *state* (state-machine progress), not just the packet trace.
//
struct StateEvent {
    cModule *module = nullptr;     // the emitting module (e.g. ...eth[0].plca)
    cModule *node = nullptr;       // containing network node, resolved from the module
    simsignal_t signal = -1;       // the signal id
    std::string signalName;        // the signal's registered name, e.g. "controlStateChanged"
    long value = 0;                // the emitted scalar (FSM state index, curID, ...)
    simtime_t time = 0;
};

} // namespace protocoltest
} // namespace inet

#endif
