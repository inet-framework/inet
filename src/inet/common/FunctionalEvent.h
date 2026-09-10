//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_FUNCTIONALEVENT_H
#define __INET_FUNCTIONALEVENT_H

#include "inet/common/INETDefs.h"

namespace inet {

using namespace std;

/**
 * Schedules a lambda function for execution at a specific simulation time.
 */
INET_API void scheduleAt(const char *name, simtime_t time, std::function<void()> f);

/**
 * Schedules a lambda function for execution after a specific simulation time.
 */
INET_API void scheduleAfter(const char *name, simtime_t delay, std::function<void()> f);

/**
 * This event executes a lambda function.
 */
class INET_API FunctionalEvent : public cEvent
{
  private:
    std::function<void()> f;

  public:
    FunctionalEvent(const char *name, std::function<void()> f) : cEvent(name), f(f) { }

    virtual cEvent *dup() const override { return new FunctionalEvent(getName(), f); }
    virtual cObject *getTargetObject() const override { return nullptr; }

    virtual void execute() override;
};

} // namespace inet

#endif

