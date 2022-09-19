//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_COMPAT_H
#define __INET_COMPAT_H

#include <omnetpp.h>

#include <iostream>

namespace inet {

// Some methods like addLifecycleListener() were moved from cEnvir to cSimulation in OMNeT++ 7.0
#if OMNETPP_BUILDNUM < 2000
#define STAGE(x) CTX_ ## x
inline omnetpp::cEnvir *getActiveSimulationOrEnvir() { return omnetpp::cSimulation::getActiveEnvir(); }
#else
#define STAGE(x) cSimulation::STAGE_ ## x
inline omnetpp::cSimulation *getActiveSimulationOrEnvir() { return omnetpp::cSimulation::getActiveSimulation(); }
#endif

#define EVSTREAM    EV

#ifdef _MSC_VER
// complementary error function, not in MSVC
double INET_API erfc(double x);

// ISO C99 function, not in MSVC
inline long lrint(double x)
{
    return (long)floor(x + 0.5);
}

// ISO C99 function, not in MSVC
inline double fmin(double a, double b)
{
    return a < b ? a : b;
}

// ISO C99 function, not in MSVC
inline double fmax(double a, double b)
{
    return a > b ? a : b;
}

#endif // _MSC_VER

} // namespace inet

#endif

