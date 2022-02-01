//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SETTABLECLOCK_H
#define __INET_SETTABLECLOCK_H

#include "inet/clock/model/OscillatorBasedClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

class INET_API SettableClock : public OscillatorBasedClock, public IScriptable
{
  protected:
    OverdueClockEventHandlingMode defaultOverdueClockEventHandlingMode = UNSPECIFIED;

  protected:
    virtual void initialize(int stage) override;

    virtual OverdueClockEventHandlingMode getOverdueClockEventHandlingMode(ClockEvent *event) const;
    virtual simtime_t handleOverdueClockEvent(ClockEvent *event, simtime_t t);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual void setClockTime(clocktime_t time, bool resetOscillator = true);
};

} // namespace inet

#endif

