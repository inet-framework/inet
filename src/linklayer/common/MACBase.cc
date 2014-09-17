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
// Author: Andras Varga (andras@omnetpp.org)
//

#include <stdlib.h>

#include "inet/linklayer/common/MACBase.h"

#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IInterfaceTable.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

MACBase::MACBase()
{
    hostModule = NULL;
    interfaceEntry = NULL;
    isOperational = false;
}

MACBase::~MACBase()
{
}

void MACBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        hostModule = findContainingNode(this);
        if (hostModule)
            hostModule->subscribe(NF_INTERFACE_DELETED, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        updateOperationalFlag(isNodeUp());    // needs to be done when interface is already registered (=last stage)
    }
}

bool MACBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER) {
            updateOperationalFlag(true);
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            updateOperationalFlag(false);
            flushQueue();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            updateOperationalFlag(false);
            clearQueue();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

void MACBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    if (signalID == NF_INTERFACE_DELETED) {
        if (interfaceEntry == check_and_cast<const InterfaceEntry *>(obj))
            interfaceEntry = NULL;
    }
}

bool MACBase::isNodeUp()
{
    NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(hostModule->getSubmodule("status"));
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void MACBase::updateOperationalFlag(bool isNodeUp)
{
    isOperational = isNodeUp;    // TODO and interface is up, too
}

void MACBase::registerInterface()    //XXX registerInterfaceIfInterfaceTableExists() ???
{
    ASSERT(interfaceEntry == NULL);
    IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (ift) {
        interfaceEntry = createInterfaceEntry();
        ift->addInterface(interfaceEntry);
    }
}

void MACBase::handleMessageWhenDown(cMessage *msg)
{
    if (isUpperMsg(msg) || msg->isSelfMessage()) {    //FIXME remove 1st part -- it is not possible to ensure that no msg is sent by upper layer (race condition!!!)
        throw cRuntimeError("Message received from higher layer while interface is off");
    }
    else {
        EV << "Interface is turned off, dropping packet\n";
        delete msg;
    }
}

} // namespace inet

