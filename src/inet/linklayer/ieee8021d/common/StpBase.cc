//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021d/common/StpBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

static const char *ENABLED_LINK_COLOR = "#000000";
static const char *DISABLED_LINK_COLOR = "#bbbbbb";
static const char *ROOT_SWITCH_COLOR = "#a5ffff";

StpBase::StpBase()
{
}

void StpBase::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER) { // "auto" MAC addresses assignment takes place in stage 0
        numPorts = ifTable->getNumInterfaces();
        switchModule->subscribe(interfaceStateChangedSignal, this);
    }

    OperationalBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        visualize = par("visualize");
        bridgePriority = par("bridgePriority");

        maxAge = par("maxAge");
        helloTime = par("helloTime");
        forwardDelay = par("forwardDelay");

        macTable.reference(this, "macTableModule", true);
        ifTable.reference(this, "interfaceTableModule", true);
        switchModule = getContainingNode(this);
    }
}

void StpBase::start()
{
    ie = chooseInterface();

    if (ie)
        bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
    else
        throw cRuntimeError("No non-loopback interface found!");
}

void StpBase::stop()
{
    ie = nullptr;
}

void StpBase::sendOut(Packet *packet, int interfaceId, const MacAddress& destAddress)
{
    packet->addTag<InterfaceReq>()->setInterfaceId(interfaceId);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::stp);
    packet->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::ieee8022llc);
    auto macAddressReq = packet->addTag<MacAddressReq>();
    macAddressReq->setSrcAddress(bridgeAddress);
    macAddressReq->setDestAddress(destAddress);
    send(packet, "relayOut");
}

void StpBase::colorLink(NetworkInterface *ie, bool forwarding) const
{
    if (visualize) {
        cGate *inGate = switchModule->gate(ie->getNodeInputGateId());
        cGate *outGate = switchModule->gate(ie->getNodeOutputGateId());
        cGate *outGateNext = outGate ? outGate->getNextGate() : nullptr;
        cGate *outGatePrev = outGate ? outGate->getPreviousGate() : nullptr;
        cGate *inGatePrev = inGate ? inGate->getPreviousGate() : nullptr;
        cGate *inGatePrev2 = inGatePrev ? inGatePrev->getPreviousGate() : nullptr;

        // TODO The Gate::getDisplayString() has a side effect: create a channel when gate currently not connected.
        //      Should check the channel existing with Gate::getChannel() before use the Gate::getDisplayString().
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
            NetworkInterface *ie = ifTable->getInterface(i);
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
            switchModule->getDisplayString().setTagArg("i", 1, ROOT_SWITCH_COLOR);
        else
            switchModule->getDisplayString().setTagArg("i", 1, "");
    }
}

const Ieee8021dInterfaceData *StpBase::getPortInterfaceData(unsigned int interfaceId) const
{
    return getPortNetworkInterface(interfaceId)->getProtocolData<Ieee8021dInterfaceData>();
}

Ieee8021dInterfaceData *StpBase::getPortInterfaceDataForUpdate(unsigned int interfaceId)
{
    return getPortNetworkInterface(interfaceId)->getProtocolDataForUpdate<Ieee8021dInterfaceData>();
}

NetworkInterface *StpBase::getPortNetworkInterface(unsigned int interfaceId) const
{
    NetworkInterface *gateIfEntry = ifTable->getInterfaceById(interfaceId);
    if (!gateIfEntry)
        throw cRuntimeError("gate's Interface is nullptr");

    return gateIfEntry;
}

int StpBase::getRootInterfaceId() const
{
    for (unsigned int i = 0; i < numPorts; i++) {
        NetworkInterface *ie = ifTable->getInterface(i);
        if (ie->getProtocolData<Ieee8021dInterfaceData>()->getRole() == Ieee8021dInterfaceData::ROOT)
            return ie->getInterfaceId();
    }

    return -1;
}

NetworkInterface *StpBase::chooseInterface()
{
    // TODO Currently, we assume that the first non-loopback interface is an Ethernet interface
    //      since STP and RSTP work on EtherSwitches.
    // NOTE that, we doesn't check if the returning interface is an Ethernet interface!
    for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
        NetworkInterface *current = ifTable->getInterface(i);
        if (!current->isLoopback())
            return current;
    }

    return nullptr;
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

