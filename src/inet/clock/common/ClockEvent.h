//
// Copyright (C) 2021 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COMMON_CLOCKEVENT_H
#define __INET_COMMON_CLOCKEVENT_H

#include "inet/clock/common/ClockEvent_m.h"

namespace inet {

class ClockBase;

class INET_API ClockEvent : public ClockEvent_Base
{
  friend ClockBase;

  protected:
    virtual void execute() override;

    void callBaseExecute() { ClockEvent_Base::execute(); }

  public:
    ClockEvent(const char *name = nullptr, short kind = 0) : ClockEvent_Base(name, kind) {}
    ClockEvent(const ClockEvent& other) : ClockEvent_Base(other) {}

    ClockEvent& operator=(const ClockEvent& other) {
        if (this == &other) return *this;
        ClockEvent_Base::operator=(other);
        return *this;
    }
    virtual ClockEvent *dup() const override { return new ClockEvent(*this); }
};

} // namespace inet

#endif

