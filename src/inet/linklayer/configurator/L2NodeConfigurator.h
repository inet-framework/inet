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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_L2NODECONFIGURATOR_H
#define __INET_L2NODECONFIGURATOR_H

#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/configurator/L2NetworkConfigurator.h"

namespace inet {

/**
 * Configures L2 data of a node. See the NED definition for details.
 */
class INET_API L2NodeConfigurator : public cSimpleModule, public ILifecycle, public cListener
{
  protected:
    NodeStatus *nodeStatus;
    IInterfaceTable *interfaceTable;
    L2NetworkConfigurator *networkConfigurator;

  public:
    L2NodeConfigurator();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
    virtual void initialize(int stage) override;

    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    virtual void prepareNode();
    virtual void prepareInterface(InterfaceEntry *interfaceEntry);
    virtual void configureNode();

    // cListener:
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG) override;
};

} // namespace inet

#endif // ifndef __INET_L2NODECONFIGURATOR_H

