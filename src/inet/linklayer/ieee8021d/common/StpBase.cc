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
// Author: Zsolt Prontvai
//

#include "inet/linklayer/ieee8021d/common/StpBase.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

static const char *ENABLED_LINK_COLOR = "#000000";
static const char *DISABLED_LINK_COLOR = "#bbbbbb";
static const char *ROOT_SWITCH_COLOR = "#a5ffff";

StpBase::StpBase()
{
}

void StpBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        visualize = par("visualize");
        bridgePriority = par("bridgePriority");

        maxAge = par("maxAge");
        helloTime = par("helloTime");
        forwardDelay = par("forwardDelay");

        macTable = getModuleFromPar<IMacAddressTable>(par("macTableModule"), this);
        ifTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        switchModule = getContainingNode(this);
    }

    if (stage == INITSTAGE_LINK_LAYER_2) {    // "auto" MAC addresses assignment takes place in stage 0
        numPorts = ifTable->getNumInterfaces();
        switchModule->subscribe(interfaceStateChangedSignal, this);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(switchModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        if (isOperational)
            start();
    }
}

void StpBase::start()
{
    isOperational = true;
    ie = chooseInterface();

    if (ie)
        bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
    else
        throw cRuntimeError("No non-loopback interface found!");
}

void StpBase::stop()
{
    isOperational = false;
    ie = nullptr;
}

void StpBase::colorLink(InterfaceEntry *ie, bool forwarding) const
{
    if (visualize) {
        cGate *inGate = switchModule->gate(ie->getNodeInputGateId());
        cGate *outGate = switchModule->gate(ie->getNodeOutputGateId());
        cGate *outGateNext = outGate ? outGate->getNextGate() : nullptr;
        cGate *outGatePrev = outGate ? outGate->getPreviousGate() : nullptr;
        cGate *inGatePrev = inGate ? inGate->getPreviousGate() : nullptr;
        cGate *inGatePrev2 = inGatePrev ? inGatePrev->getPreviousGate() : nullptr;

        if (outGate && inGate && inGatePrev && outGateNext && outGatePrev && inGatePrev2) {
            if (forwarding) {
                outGatePrev->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
            }
            else {
                outGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }

            if ((!inGatePrev2->getDisplayString().containsTag("ls") || strcmp(inGatePrev2->getDisplayString().getTagArg("ls", 0), ENABLED_LINK_COLOR) == 0) && forwarding) {
                outGate->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
                inGatePrev->getDisplayString().setTagArg("ls", 0, ENABLED_LINK_COLOR);
            }
            else {
                outGate->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
                inGatePrev->getDisplayString().setTagArg("ls", 0, DISABLED_LINK_COLOR);
            }
        }
    }
}

void StpBase::refreshDisplay() const
{
    if (visualize) {
        for (unsigned int i = 0; i < numPorts; i++) {
            InterfaceEntry *ie = ifTable->getInterface(i);
            cModule *nicModule = ie;
            if (isOperational) {
                const Ieee8021dInterfaceData *port = getPortInterfaceData(ie->getInterfaceId());

                // color link
                colorLink(ie, isOperational && (port->getState() == Ieee8021dInterfaceData::FORWARDING));

                // label ethernet interface with port status and role
                if (nicModule != nullptr) {
                    char buf[32];
                    sprintf(buf, "%s\n%s", port->getRoleName(), port->getStateName());
                    nicModule->getDisplayString().setTagArg("t", 0, buf);
                }
            }
            else {
                // color link
                colorLink(ie, false);

                // label ethernet interface with port status and role
                if (nicModule != nullptr) {
                    nicModule->getDisplayString().setTagArg("t", 0, "");
                }
            }
        }

        // mark root switch
        if (isOperational && getRootInterfaceId() == -1)
            switchModule->getDisplayString().setTagArg("i", 1, ROOT_SWITCH_COLOR);
        else
            switchModule->getDisplayString().setTagArg("i", 1, "");
    }
}

Ieee8021dInterfaceData *StpBase::getPortInterfaceData(unsigned int interfaceId)
{
    Ieee8021dInterfaceData *portData = getPortInterfaceEntry(interfaceId)->ieee8021dData();
    if (!portData)
        throw cRuntimeError("Ieee8021dInterfaceData not found!");

    return portData;
}

InterfaceEntry *StpBase::getPortInterfaceEntry(unsigned int interfaceId)
{
    InterfaceEntry *gateIfEntry = ifTable->getInterfaceById(interfaceId);
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");

    return gateIfEntry;
}

int StpBase::getRootInterfaceId() const
{
    for (unsigned int i = 0; i < numPorts; i++) {
        InterfaceEntry *ie = ifTable->getInterface(i);
        if (ie->ieee8021dData()->getRole() == Ieee8021dInterfaceData::ROOT)
            return ie->getInterfaceId();
    }

    return -1;
}

InterfaceEntry *StpBase::chooseInterface()
{
    // TODO: Currently, we assume that the first non-loopback interface is an Ethernet interface
    //       since STP and RSTP work on EtherSwitches.
    //       NOTE that, we doesn't check if the returning interface is an Ethernet interface!
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        InterfaceEntry *current = ifTable->getInterface(i);
        if (!current->isLoopback())
            return current;
    }

    return nullptr;
}

bool StpBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if (static_cast<NodeStartOperation::Stage>(stage) == NodeStartOperation::STAGE_LINK_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (static_cast<NodeShutdownOperation::Stage>(stage) == NodeShutdownOperation::STAGE_LINK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (static_cast<NodeCrashOperation::Stage>(stage) == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());

    return true;
}

} // namespace inet

