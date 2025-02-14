//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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

/**
 * Schedules this timer to fire at the given time. Does not touch any older schedules of this timer.
 * \param The time when the timer shall fire.
 */
void Timer::scheduleAt(simtime_t time)
{
    quicSimpleMod->scheduleAt(time, timerMessage);
}

/**
 * Returns true if the timer is currently scheduled, false otherwise.
 */
bool Timer::isScheduled()
{
    return timerMessage->isScheduled();
}

/**
 * Updates the schedule of this timer. Cancels and old schedule before, if there is one.
 * \param The new time when the timer shall fire.
 */
void Timer::update(simtime_t time)
{
    if (timerMessage->isScheduled()) {
        cancel();
    }
    scheduleAt(time);
}

/**
 * Cancel the schedule of this timer.
 */
void Timer::cancel()
{
    quicSimpleMod->cancelEvent(timerMessage);
}

} /* namespace quic */
} /* namespace inet */
