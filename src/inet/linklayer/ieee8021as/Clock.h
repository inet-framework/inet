//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef __IEEE8021AS_CLOCK_H_
#define __IEEE8021AS_CLOCK_H_

#include <omnetpp.h>

using namespace omnetpp;


class Clock : public cSimpleModule
{
    SimTime lastAdjustedClock;
    SimTime lastSimTime;
    SimTime clockDrift;

  protected:
    virtual void initialize();

  public:
    SimTime getCurrentTime();
    SimTime getCalculatedDrift(SimTime value);
    void adjustTime(SimTime value);
};

#endif
