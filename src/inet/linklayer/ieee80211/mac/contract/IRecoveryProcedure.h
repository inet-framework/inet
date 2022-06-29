//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IRECOVERYPROCEDURE_H
#define __INET_IRECOVERYPROCEDURE_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace ieee80211 {

class INET_API IRecoveryProcedure
{
  public:
    static simsignal_t contentionWindowChangedSignal;
    static simsignal_t retryLimitReachedSignal;

  public:
    class INET_API ICwCalculator {
      public:
        virtual ~ICwCalculator() {}

        virtual void incrementCw() = 0;
        virtual void resetCw() = 0;
        virtual int getCw() = 0;
    };

  public:
    virtual ~IRecoveryProcedure() {}
};

} // namespace ieee80211
} // namespace inet

#endif

