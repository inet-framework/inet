//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef __INET_TRACITESTAPP_H
#define __INET_TRACITESTAPP_H

#include <set>
#include <list>

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/mobility/group/TraCIMobility.h"

namespace inet {

/**
 * FIXME
 */
class TraCITestApp : public cSimpleModule, protected cListener, public ILifecycle
{
  protected:
    // parameter
    int testNumber;

    // state
    TraCIMobility *traci;
    std::set<std::string> visitedEdges;    // set of edges this vehicle visited
    bool hasStopped;    // true if at some point in time this vehicle travelled at negligible speed
    static simsignal_t mobilityStateChangedSignal;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void finish();

    void handleSelfMsg(cMessage *msg);
    void handleLowerMsg(cMessage *msg);
    virtual void handleMessage(cMessage *msg);
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);

    void handlePositionUpdate();

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
    { Enter_Method_Silent(); throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName()); return true; }

  public:
    TraCITestApp() { traci = NULL; }
};

} // namespace inet

#endif // ifndef __INET_TRACITESTAPP_H

