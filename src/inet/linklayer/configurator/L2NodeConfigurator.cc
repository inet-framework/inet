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

#include "inet/linklayer/configurator/L2NodeConfigurator.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"

namespace inet {

Define_Module(L2NodeConfigurator);

L2NodeConfigurator::L2NodeConfigurator()
{
    nodeStatus = nullptr;
    interfaceTable = nullptr;
    networkConfigurator = nullptr;
}

void L2NodeConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        cModule *host = getContainingNode(this);
        nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
        interfaceTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        networkConfigurator = findModuleFromPar<L2NetworkConfigurator>(par("l2ConfiguratorModule"), this);
        host->subscribe(NF_INTERFACE_CREATED, this);
    }
}

bool L2NodeConfigurator::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            prepareNode();
            configureNode();
        }
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

void L2NodeConfigurator::prepareInterface(InterfaceEntry *interfaceEntry)
{
    ASSERT(!interfaceEntry->ieee8021dData());
    Ieee8021dInterfaceData *ieee8021dInterfaceData = new Ieee8021dInterfaceData();
    interfaceEntry->setIeee8021dInterfaceData(ieee8021dInterfaceData);
}

void L2NodeConfigurator::configureNode()
{
    ASSERT(networkConfigurator);
    // std::cout << "configureNode(): " << interfaceTable->getNumInterfaces() << endl;
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++)
        networkConfigurator->configureInterface(interfaceTable->getInterface(i));
}

void L2NodeConfigurator::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    if (nodeStatus && nodeStatus->getState() != NodeStatus::UP)
        return;

    if (signalID == NF_INTERFACE_CREATED) {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(obj);
        prepareInterface(ie);
        if (networkConfigurator)
            networkConfigurator->configureInterface(ie);
    }
}

} // namespace inet

