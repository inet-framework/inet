//
// Copyright (C) 2020 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

