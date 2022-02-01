//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CONTRACT_CLOCKEVENT_H
#define __INET_CONTRACT_CLOCKEVENT_H

#include "inet/common/INETDefs.h"

namespace inet {

#ifdef INET_WITH_CLOCK
class ClockEvent;
#else
typedef cMessage ClockEvent;
#endif

} // namespace inet

#endif

