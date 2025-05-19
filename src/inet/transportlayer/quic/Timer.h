//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef INET_APPLICATIONS_QUIC_TIMER_H_
#define INET_APPLICATIONS_QUIC_TIMER_H_

#include "Quic.h"

namespace inet {
namespace quic {

class Timer {
public:

    Timer(Quic *quicSimpleMod, cMessage *msg);
    virtual ~Timer();

    /**
     * Schedules this timer to fire at the given time. Does not touch any older schedules of this timer.
     * @param The time when the timer shall fire.
     */
    void scheduleAt(simtime_t time);

    /**
     * Returns true if the timer is currently scheduled, false otherwise.
     */
    bool isScheduled();

    /**
     * Updates the schedule of this timer. Cancels and old schedule before, if there is one.
     * @param The new time when the timer shall fire.
     */
    void update(simtime_t time);

    /**
     * Cancel the schedule of this timer.
     */
    void cancel();

    cMessage *getTimerMessage() {
        return timerMessage;
    }

private:
    Quic *quicSimpleMod;
    cMessage *timerMessage;
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_APPLICATIONS_QUIC_TIMER_H_ */
