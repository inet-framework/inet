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

#include "L2NodeConfigurator.h"
#include "ModuleAccess.h"
#include "InterfaceTableAccess.h"
#include "RoutingTableAccess.h"
#include "NodeStatus.h"
#include "NodeOperations.h"
#include "opp_utils.h"   // for OPP_Global::getModuleByPath()

Define_Module(L2NodeConfigurator);

L2NodeConfigurator::L2NodeConfigurator()
{
    nodeStatus = NULL;
    interfaceTable = NULL;
    networkConfigurator = NULL;
}

void L2NodeConfigurator::initialize(int stage)
{
    if (stage == 0)
    {
        const char * networkConfiguratorPath = par("l2ConfiguratorModule");
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        interfaceTable = InterfaceTableAccess().get();

        if (!networkConfiguratorPath[0])
            networkConfigurator = NULL;
        else {
#if OMNETPP_VERSION < 0x0403
            cModule * module = OPP_Global::getModuleByPath(this, networkConfiguratorPath);  // compatibility
#else
            cModule * module = getModuleByPath(networkConfiguratorPath);
#endif
            if (!module)
                throw cRuntimeError("Configurator module '%s' not found (check the 'networkConfiguratorModule' parameter)", networkConfiguratorPath);
            networkConfigurator = check_and_cast<L2NetworkConfigurator *>(module);
        }
    }
    else if (stage == 1)
    {
        if (!nodeStatus || nodeStatus->getState() == NodeStatus::UP)
            prepareNode();
    }
    else if (stage == 2)
    {
        if ((!nodeStatus || nodeStatus->getState() == NodeStatus::UP) && networkConfigurator)
            configureNode();
    }
}

bool L2NodeConfigurator::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER)
            prepareNode();
        else if (stage == NodeStartOperation::STAGE_NETWORK_LAYER && networkConfigurator)
            configureNode();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
        /*nothing to do*/;
    else if (dynamic_cast<NodeCrashOperation *>(operation))
        /*nothing to do*/;
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void L2NodeConfigurator::prepareNode()
{
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        prepareInterface(interfaceTable->getInterface(i));
}

void L2NodeConfigurator::prepareInterface(InterfaceEntry * interfaceEntry)
{
    ASSERT(!interfaceEntry->vlanData());
    VLANInterfaceData * vlanInterfaceData = new VLANInterfaceData();
    interfaceEntry->setVLANInterfaceData(vlanInterfaceData);
}

void L2NodeConfigurator::configureNode()
{
    ASSERT(networkConfigurator);
    // std::cout << "configureNode(): " << interfaceTable->getNumInterfaces() << endl;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        networkConfigurator->configureInterface(interfaceTable->getInterface(i));
}
