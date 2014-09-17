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
// Author: Levente Meszaros (levy@omnetpp.org)
//

#ifndef __INET_NODESTATUS_H
#define __INET_NODESTATUS_H

#include "inet/common/lifecycle/ILifecycle.h"

namespace inet {

/**
 * Keeps track of the status of network node (up, down, etc.) for other
 * modules, and also displays it as a small overlay icon on this module
 * and on the module of the network node.
 *
 * Other modules can obtain the network node's status by calling the
 * getState() method.
 *
 * See NED file for more information.
 */
class INET_API NodeStatus : public cSimpleModule, public ILifecycle
{
  public:
    enum State { UP, DOWN, GOING_UP, GOING_DOWN };
    static simsignal_t nodeStatusChangedSignal;    // the signal used to notify subscribers about status changes

  private:
    State state;
    std::string origIcon;

  public:
    virtual State getState() const { return state; }

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle messages"); }
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    virtual void setState(State state);
    virtual void updateDisplayString();
    static State getStateByName(const char *name);
};

} // namespace inet

#endif // ifndef __INET_NODESTATUS_H

