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

#ifndef __INET_INTERFACEOPERATIONS_H_
#define __INET_INTERFACEOPERATIONS_H_

#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

class InterfaceEntry;

/**
 * Base class for lifecycle operations that manipulate a network interface.
 */
class INET_API InterfaceOperationBase : public LifecycleOperation
{
  public:
    enum Stage { STAGE_LOCAL, STAGE_LAST };
  private:
    InterfaceEntry *ie; // the interface to be operated on
  public:
    InterfaceOperationBase() : ie(nullptr) {}
    virtual void initialize(cModule *module, StringMap& params);
    virtual int getNumStages() const {return STAGE_LAST+1;}
    InterfaceEntry *getInterface() const {return ie;}
};

/**
 * Lifecycle operation to bring up a network interface.
 */
class INET_API InterfaceUpOperation : public InterfaceOperationBase
{
};

/**
 * Lifecycle operation to bring down a network interface.
 */
class INET_API InterfaceDownOperation : public InterfaceOperationBase
{
};

} // namespace inet

#endif // #ifndef __INET_INTERFACEOPERATIONS_H_

