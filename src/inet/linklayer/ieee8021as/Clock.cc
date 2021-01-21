//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

#include "Clock.h"

Define_Module(Clock);

void Clock::initialize()
{
    lastAdjustedClock = 0;
    lastSimTime = 0;
    clockDrift = par("clockDrift");
}

/* Return drifted current time based on last adjusted time */
SimTime Clock::getCurrentTime()
{
    SimTime duration = simTime() - lastSimTime;
    SimTime currentTime = lastAdjustedClock + duration + duration.dbl()*clockDrift.dbl()/1000000;

    return currentTime;
}

/* Return drift for only the given value */
SimTime Clock::getCalculatedDrift(SimTime value)
{
    SimTime drift = clockDrift*value.dbl()/1000000;

    return drift;
}

/* Adjust time based on last adjusted time */
void Clock::adjustTime(SimTime value)
{
    lastAdjustedClock = value;
    lastSimTime = simTime();
}
