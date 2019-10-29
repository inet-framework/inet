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

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/ieee8021d/common/StpBase.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

StpBase::StpBase()
{

}

void StpBase::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) {    // "auto" MAC addresses assignment takes place in stage 0
        numPorts = ifTable->getNumInterfaces();
        switchModule->subscribe(interfaceStateChangedSignal, this);
    }

    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        visualize = par("visualize");
        colorLinkEnabled = par("colorLinkEnabled").stdstringValue();
        colorLinkDisabled = par("colorLinkDisabled").stdstringValue();
        colorRootBridge = par("colorRootBridge").stdstringValue();

        bridgePriority = par("bridgePriority");

        maxAge = par("maxAge");
        helloTime = par("helloTime");
        forwardDelay = par("forwardDelay");

        macTable = getModuleFromPar<IMacAddressTable>(par("macTableModule"), this);
        ifTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        switchModule = getContainingNode(this);
    }
}

void StpBase::start()
{
    bridgeAddress = ifTable->getBaseMacAddress();
    ASSERT(bridgeAddress != MacAddress::UNSPECIFIED_ADDRESS);
}

void StpBase::stop()
{

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
                outGatePrev->getDisplayString().setTagArg("ls", 0, colorLinkEnabled.c_str());
                inGate->getDisplayString().setTagArg("ls", 0, colorLinkEnabled.c_str());
            }
            else {
                outGatePrev->getDisplayString().setTagArg("ls", 0, colorLinkDisabled.c_str());
                inGate->getDisplayString().setTagArg("ls", 0, colorLinkDisabled.c_str());
            }

            if ((!inGatePrev2->getDisplayString().containsTag("ls") || strcmp(inGatePrev2->getDisplayString().getTagArg("ls", 0), colorLinkEnabled.c_str()) == 0) && forwarding) {
                outGate->getDisplayString().setTagArg("ls", 0, colorLinkEnabled.c_str());
                inGatePrev->getDisplayString().setTagArg("ls", 0, colorLinkEnabled.c_str());
            }
            else {
                outGate->getDisplayString().setTagArg("ls", 0, colorLinkDisabled.c_str());
                inGatePrev->getDisplayString().setTagArg("ls", 0, colorLinkDisabled.c_str());
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
            if (isUp()) {
                const Ieee8021dInterfaceData *port = getPortInterfaceData(ie->getInterfaceId());

                // color link
                colorLink(ie, isUp() && (port->getState() == Ieee8021dInterfaceData::FORWARDING));

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
        if (isUp() && getRootInterfaceId() == -1)
            switchModule->getDisplayString().setTagArg("i", 1, colorRootBridge.c_str());
        else
            switchModule->getDisplayString().setTagArg("i", 1, "");
    }
}

Ieee8021dInterfaceData *StpBase::getPortInterfaceData(unsigned int interfaceId)
{
    Ieee8021dInterfaceData *portData = getPortInterfaceEntry(interfaceId)->getProtocolData<Ieee8021dInterfaceData>();
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
        if (ie->getProtocolData<Ieee8021dInterfaceData>()->getRole() == Ieee8021dInterfaceData::ROOT)
            return ie->getInterfaceId();
    }

    return -1;
}

void StpBase::handleStartOperation(LifecycleOperation *operation)
{
    start();
}

void StpBase::handleStopOperation(LifecycleOperation *operation)
{
    stop();
}

void StpBase::handleCrashOperation(LifecycleOperation *operation)
{
    stop();
}

} // namespace inet

