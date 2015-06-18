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

#include "inet/linklayer/ieee8021d/common/STPBase.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

static const char *ENABLED_LINK_COLOR = "#000000";
static const char *DISABLED_LINK_COLOR = "#bbbbbb";
static const char *ROOT_SWITCH_COLOR = "#a5ffff";

STPBase::STPBase()
{
}

void STPBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        visualize = par("visualize");
        bridgePriority = par("bridgePriority");

        maxAge = par("maxAge");
        helloTime = par("helloTime");
        forwardDelay = par("forwardDelay");

        macTable = check_and_cast<IMACAddressTable *>(getModuleByPath(par("macTablePath")));
        ifTable = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));
        switchModule = findContainingNode(this);
        if (!switchModule)
            throw cRuntimeError("Containing @networkNode module not found");
        numPorts = switchModule->gate("ethg$o", 0)->getVectorSize();
    }

    if (stage == INITSTAGE_LINK_LAYER_2) {    // "auto" MAC addresses assignment takes place in stage 0
        cModule *m = findContainingNode(this);
        if (m)
            m->subscribe(NF_INTERFACE_STATE_CHANGED, this);

        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(switchModule->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        if (isOperational)
            start();
    }
}

void STPBase::start()
{
    isOperational = true;
    ie = chooseInterface();

    if (ie)
        bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
    else
        throw cRuntimeError("No non-loopback interface found!");
}

void STPBase::stop()
{
    isOperational = false;
    // colors all connected link gray
    for (unsigned int i = 0; i < numPorts; i++)
        colorLink(i, false);
    switchModule->getDisplayString().setTagArg("i", 1, "");
    ie = nullptr;
}

void STPBase::colorLink(unsigned int i, bool forwarding)
{
    if (hasGUI() && visualize) {
        cGate *inGate = switchModule->gate("ethg$i", i);
        cGate *outGate = switchModule->gate("ethg$o", i);
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

void STPBase::updateDisplay()
{
    if (hasGUI() && visualize) {
        for (unsigned int i = 0; i < numPorts; i++) {
            Ieee8021dInterfaceData *port = getPortInterfaceData(i);

            // color link
            colorLink(i, port->getState() == Ieee8021dInterfaceData::FORWARDING);

            // label ethernet interface with port status and role
            cModule *nicModule = switchModule->getSubmodule("eth", i);
            if (nicModule != nullptr) {
                char buf[32];
                sprintf(buf, "%s\n%s", port->getRoleName(), port->getStateName());
                nicModule->getDisplayString().setTagArg("t", 0, buf);
            }
        }

        // mark root switch
        if (getRootIndex() == -1)
            switchModule->getDisplayString().setTagArg("i", 1, ROOT_SWITCH_COLOR);
        else
            switchModule->getDisplayString().setTagArg("i", 1, "");
    }
}

Ieee8021dInterfaceData *STPBase::getPortInterfaceData(unsigned int portNum)
{
    Ieee8021dInterfaceData *portData = getPortInterfaceEntry(portNum)->ieee8021dData();
    if (!portData)
        throw cRuntimeError("Ieee8021dInterfaceData not found!");

    return portData;
}

InterfaceEntry *STPBase::getPortInterfaceEntry(unsigned int portNum)
{
    cGate *gate = switchModule->gate("ethg$o", portNum);
    if (!gate)
        throw cRuntimeError("gate is nullptr");
    InterfaceEntry *gateIfEntry = ifTable->getInterfaceByNodeOutputGateId(gate->getId());
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");

    return gateIfEntry;
}

int STPBase::getRootIndex()
{
    for (unsigned int i = 0; i < numPorts; i++)
        if (getPortInterfaceData(i)->getRole() == Ieee8021dInterfaceData::ROOT)
            return i;

    return -1;
}

InterfaceEntry *STPBase::chooseInterface()
{
    // TODO: Currently, we assume that the first non-loopback interface is an Ethernet interface
    //       since STP and RSTP work on EtherSwitches.
    //       NOTE that, we doesn't check if the returning interface is an Ethernet interface!
    IInterfaceTable *ift = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *current = ift->getInterface(i);
        if (!current->isLoopback())
            return current;
    }

    return nullptr;
}

bool STPBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER)
            start();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER)
            stop();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            stop();
    }
    else
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());

    return true;
}

} // namespace inet

