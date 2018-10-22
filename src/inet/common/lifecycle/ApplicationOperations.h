//
// Copyright (C) Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_APPLICATIONOPERATIONS_H_
#define __INET_APPLICATIONOPERATIONS_H_

#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * Base class for lifecycle operations that manipulate a application.
 */
class INET_API ApplicationOperationBase : public LifecycleOperation
{
  public:
    enum Stage { STAGE_LOCAL, STAGE_LAST };

  public:
    ApplicationOperationBase() {}
    virtual void initialize(cModule *module, StringMap& params);
    virtual int getNumStages() const { return STAGE_LAST + 1; }
};

/**
 * Lifecycle operation to start an application.
 */
class INET_API ApplicationStartOperation : public ApplicationOperationBase
{
};

/**
 * Lifecycle operation to stop an application.
 */
class INET_API ApplicationStopOperation : public ApplicationOperationBase
{
};

} // namespace inet

#endif // #ifndef __INET_APPLICATIONOPERATIONS_H_

