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

#ifndef __INET_L2NODECONFIGURATOR_H_
#define __INET_L2NODECONFIGURATOR_H_

#include "INETDefs.h"
#include "ILifecycle.h"
#include "NodeStatus.h"
#include "IInterfaceTable.h"
#include "L2NetworkConfigurator.h"

/**
 * Configures L2 data of a node. See the NED definition for details.
 */
class L2NodeConfigurator : public cSimpleModule, public ILifecycle {
    protected:
        NodeStatus * nodeStatus;
        IInterfaceTable * interfaceTable;
        L2NetworkConfigurator * networkConfigurator;

    public:
        L2NodeConfigurator();

    protected:
        virtual int numInitStages() const { return 4; }
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("this module doesn't handle messages, it runs only in initialize()"); }
        virtual void initialize(int stage);
        virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
        virtual void prepareNode();
        virtual void prepareInterface(InterfaceEntry *interfaceEntry);
        virtual void configureNode();
};

#endif
