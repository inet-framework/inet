//
// Copyright (C) 2019-2024 Timo Völker, Ekaterina Volodina
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "Timer.h"

namespace inet {
namespace quic {

Timer::Timer(Quic *quicSimpleMod, cMessage *msg) {
    this->quicSimpleMod = quicSimpleMod;
    timerMessage = msg;
}

Timer::~Timer() {
    quicSimpleMod->cancelAndDelete(timerMessage);
}

void Timer::scheduleAt(simtime_t time)
{
    quicSimpleMod->scheduleAt(time, timerMessage);
}

bool Timer::isScheduled()
{
    return timerMessage->isScheduled();
}

void Timer::update(simtime_t time)
{
    if (timerMessage->isScheduled()) {
        cancel();
    }
    scheduleAt(time);
}

void Timer::cancel()
{
    quicSimpleMod->cancelEvent(timerMessage);
}

} /* namespace quic */
} /* namespace inet */
