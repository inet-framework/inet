//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ITOKENSTORAGE_H
#define __INET_ITOKENSTORAGE_H

#include "inet/common/INETDefs.h"

namespace inet {
namespace queueing {

/**
 * This class defines the interface for token storages.
 */
class INET_API ITokenStorage
{
  public:
    virtual ~ITokenStorage() {}

    virtual double getNumTokens() const = 0;

    virtual void addTokens(double numTokens) = 0;

    virtual void removeTokens(double numTokens) = 0;

    virtual void addTokenProductionRate(double tokenRate) = 0;

    virtual void removeTokenProductionRate(double tokenRate) = 0;
};

} // namespace queueing
} // namespace inet

#endif

