//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.clock.contract.ClockTime;
import inet.clock.contract.IClock;

namespace inet;

cplusplus {{
class ClockBase;
class OscillatorBasedClock;
class SettableClock;
}}

enum OverdueClockEventHandlingMode {
    UNSPECIFIED = -1;
    EXECUTE = 0;
    SKIP    = 1;
    ERROR   = 2;
}

message ClockEvent
{
    IClock *clock;
    bool relative = false;
    clocktime_t arrivalClockTime = -1;
    uint64_t insertionOrder = 0;
    OverdueClockEventHandlingMode overdueClockEventHandlingMode = UNSPECIFIED;
}

cplusplus(ClockEvent) {{
    friend ClockBase;
    friend OscillatorBasedClock;
    friend SettableClock;

  protected:
    virtual void execute() override;
    void callBaseExecute() { cMessage::execute(); }
}}
