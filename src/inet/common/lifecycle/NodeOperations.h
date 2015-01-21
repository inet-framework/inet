//
// Copyright (C) 2013 Opensim Ltd.
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
// Author: Levente Meszaros <levy@omnetpp.org>, Andras Varga (andras@omnetpp.org)
//

#ifndef __INET_NODEOPERATIONS_H
#define __INET_NODEOPERATIONS_H

#include "inet/common/lifecycle/LifecycleOperation.h"

namespace inet {

/**
 * Base class for operations that manipulate network nodes.
 */
class INET_API NodeOperation : public LifecycleOperation
{
  public:
    virtual void initialize(cModule *module, StringMap& params) override;
};

/**
 * This operation represents the process of turning on a network node
 * after a shutdown, crash or suspend operation.
 *
 * The operation should be applied to the module of a network node. Operation
 * stages are organized bottom-up similarly to the OSI network layers.
 */
class INET_API NodeStartOperation : public NodeOperation
{
  public:
    enum Stage {
        STAGE_LOCAL,    // for changes that don't depend on other modules
        STAGE_PHYSICAL_LAYER,
        STAGE_LINK_LAYER,
        STAGE_NETWORK_LAYER,
        STAGE_TRANSPORT_LAYER,
        STAGE_ROUTING_PROTOCOLS,
        STAGE_APPLICATION_LAYER,
        STAGE_LAST
    };

  public:
    virtual int getNumStages() const override { return STAGE_LAST + 1; }
    virtual Kind getKind() const override { return UP; }
};

/**
 * This operation represents the process of orderly shutting down a network node.
 *
 * The operation should be applied to the module of a network node. Operation
 * stages are organized top-down similarly to the OSI network layers.
 */
class INET_API NodeShutdownOperation : public NodeOperation
{
  public:
    enum Stage {
        STAGE_LOCAL,    // for changes that don't depend on other modules
        STAGE_APPLICATION_LAYER,
        STAGE_ROUTING_PROTOCOLS,
        STAGE_TRANSPORT_LAYER,
        STAGE_NETWORK_LAYER,
        STAGE_LINK_LAYER,
        STAGE_PHYSICAL_LAYER,
        STAGE_LAST    // for changes that others shouldn't depend on
    };

  public:
    virtual int getNumStages() const override { return STAGE_LAST + 1; }
    virtual Kind getKind() const override { return DOWN; }
};

/** TODO:
 * This operation represents the process of suspending (hybernating)
 * a network node. All state information (routing tables, etc) will
 * remain intact, but the node will stop responding to messages.
 *
 * The operation should be applied to the module of a network node. Operation
 * stages are organized top-down similarly to the OSI network layers.
 * /
   class INET_API NodeSuspendOperation : public NodeOperation {
   public:
    enum Stage {
      STAGE_LOCAL, // for changes that don't depend on other modules
      STAGE_APPLICATION_LAYER,
      STAGE_TRANSPORT_LAYER,
      STAGE_NETWORK_LAYER,
      STAGE_LINK_LAYER,
      STAGE_PHYSICAL_LAYER,
      STAGE_LAST
    };

   public:
    virtual int getNumStages() const { return STAGE_LAST+1; }
    virtual Kind getKind() const { return DOWN; }
   };
 */

/**
 * This operation represents the process of crashing a network node. The
 * difference between this operation and NodeShutdownOperation is that the
 * network node will not do a graceful shutdown (e.g. routing protocols will
 * not have chance of notifying peers about broken routes).
 *
 * The operation should be applied to the module of a network node. The
 * operation has only one stage, and the execution must finish in zero
 * simulation time.
 */
class INET_API NodeCrashOperation : public NodeOperation
{
  public:
    enum Stage {
        STAGE_CRASH,    // the only stage, must execute within zero simulation time
        STAGE_LAST
    };

  public:
    virtual int getNumStages() const override { return STAGE_LAST + 1; }
    virtual Kind getKind() const override { return DOWN; }
};

} // namespace inet

#endif // ifndef __INET_NODEOPERATIONS_H

