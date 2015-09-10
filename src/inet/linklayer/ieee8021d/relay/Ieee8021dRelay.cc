// Copyright (C) 2013 OpenSim Ltd.
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
// Author: Benjamin Martin Seregi

#include "inet/linklayer/ieee8021d/relay/Ieee8021dRelay.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/configurator/Ieee8021dInterfaceData.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(Ieee8021dRelay);

Ieee8021dRelay::Ieee8021dRelay()
{
}

void Ieee8021dRelay::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        // statistics
        numDispatchedBDPUFrames = numDispatchedNonBPDUFrames = numDeliveredBDPUsToSTP = 0;
        numReceivedBPDUsFromSTP = numReceivedNetworkFrames = numDroppedFrames = 0;

        // number of ports
        portCount = gate("ifOut", 0)->size();
        if (gate("ifIn", 0)->size() != (int)portCount)
            throw cRuntimeError("the sizes of the ifIn[] and ifOut[] gate vectors must be the same");
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        macTable = check_and_cast<IMACAddressTable *>(getModuleByPath(par("macTablePath")));
        ifTable = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));

        if (isOperational) {
            ie = chooseInterface();

            if (ie)
                bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
            else
                throw cRuntimeError("No non-loopback interface found!");
        }

        isStpAware = gate("stpIn")->isConnected();    // if the stpIn is not connected then the switch is STP/RSTP unaware

        WATCH(bridgeAddress);
        WATCH(numReceivedNetworkFrames);
        WATCH(numDroppedFrames);
        WATCH(numReceivedBPDUsFromSTP);
        WATCH(numDeliveredBDPUsToSTP);
        WATCH(numDispatchedNonBPDUFrames);
    }
}

void Ieee8021dRelay::handleMessage(cMessage *msg)
{
    if (!isOperational) {
        EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropped it." << endl;
        delete msg;
        return;
    }

    if (!msg->isSelfMessage()) {
        // messages from STP process
        if (strcmp(msg->getArrivalGate()->getName(), "stpIn") == 0) {
            numReceivedBPDUsFromSTP++;
            EV_INFO << "Received " << msg << " from STP/RSTP module." << endl;
            BPDU *bpdu = check_and_cast<BPDU *>(msg);
            dispatchBPDU(bpdu);
        }
        // messages from network
        else if (strcmp(msg->getArrivalGate()->getName(), "ifIn") == 0) {
            numReceivedNetworkFrames++;
            EV_INFO << "Received " << msg << " from network." << endl;
            EtherFrame *frame = check_and_cast<EtherFrame *>(msg);
            handleAndDispatchFrame(frame);
        }
    }
    else
        throw cRuntimeError("This module doesn't handle self-messages!");
}

void Ieee8021dRelay::broadcast(EtherFrame *frame)
{
    EV_DETAIL << "Broadcast frame " << frame << endl;

    unsigned int arrivalGate = frame->getArrivalGate()->getIndex();

    for (unsigned int i = 0; i < portCount; i++)
        if (i != arrivalGate && (!isStpAware || getPortInterfaceData(i)->isForwarding()))
            dispatch(frame->dup(), i);


    delete frame;
}

void Ieee8021dRelay::handleAndDispatchFrame(EtherFrame *frame)
{
    int arrivalGate = frame->getArrivalGate()->getIndex();
    Ieee8021dInterfaceData *arrivalPortData = getPortInterfaceData(arrivalGate);
    learn(frame);

    // BPDU Handling
    if ((frame->getDest() == MACAddress::STP_MULTICAST_ADDRESS || frame->getDest() == bridgeAddress) && arrivalPortData->getRole() != Ieee8021dInterfaceData::DISABLED) {
        EV_DETAIL << "Deliver BPDU to the STP/RSTP module" << endl;
        deliverBPDU(frame);    // deliver to the STP/RSTP module
    }
    else if (isStpAware && !arrivalPortData->isForwarding()) {
        EV_INFO << "The arrival port is not forwarding! Discarding it!" << endl;
        numDroppedFrames++;
        delete frame;
    }
    else if (frame->getDest().isBroadcast()) {    // broadcast address
        broadcast(frame);
    }
    else {
        int outGate = macTable->getPortForAddress(frame->getDest());
        // Not known -> broadcast
        if (outGate == -1) {
            EV_DETAIL << "Destination address = " << frame->getDest() << " unknown, broadcasting frame " << frame << endl;
            broadcast(frame);
        }
        else {
            if (outGate != arrivalGate) {
                Ieee8021dInterfaceData *outPortData = getPortInterfaceData(outGate);

                if (!isStpAware || outPortData->isForwarding())
                    dispatch(frame, outGate);
                else {
                    EV_INFO << "Output port " << outGate << " is not forwarding. Discarding!" << endl;
                    numDroppedFrames++;
                    delete frame;
                }
            }
            else {
                EV_DETAIL << "Output port is same as input port, " << frame->getFullName() << " destination = " << frame->getDest() << ", discarding frame " << frame << endl;
                numDroppedFrames++;
                delete frame;
            }
        }
    }
}

void Ieee8021dRelay::dispatch(EtherFrame *frame, unsigned int portNum)
{
    EV_INFO << "Sending frame " << frame << " on output port " << portNum << "." << endl;

    if (portNum >= portCount)
        throw cRuntimeError("Output port %d doesn't exist!", portNum);

    EV_INFO << "Sending " << frame << " with destination = " << frame->getDest() << ", port = " << portNum << endl;

    numDispatchedNonBPDUFrames++;
    send(frame, "ifOut", portNum);
}

void Ieee8021dRelay::learn(EtherFrame *frame)
{
    int arrivalGate = frame->getArrivalGate()->getIndex();
    Ieee8021dInterfaceData *port = getPortInterfaceData(arrivalGate);

    if (!isStpAware || port->isLearning())
        macTable->updateTableWithAddress(arrivalGate, frame->getSrc());
}

void Ieee8021dRelay::dispatchBPDU(BPDU *bpdu)
{
    Ieee802Ctrl *controlInfo = check_and_cast<Ieee802Ctrl *>(bpdu->removeControlInfo());
    unsigned int portNum = controlInfo->getSwitchPort();
    MACAddress address = controlInfo->getDest();
    delete controlInfo;

    if (portNum >= portCount)
        throw cRuntimeError("Output port %d doesn't exist!", portNum);

    // TODO: use LLCFrame
    EthernetIIFrame *frame = new EthernetIIFrame(bpdu->getName());

    frame->setKind(bpdu->getKind());
    frame->setSrc(bridgeAddress);
    frame->setDest(address);
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    frame->setEtherType(-1);
    frame->encapsulate(bpdu);

    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);

    EV_INFO << "Sending BPDU frame " << frame << " with destination = " << frame->getDest() << ", port = " << portNum << endl;
    numDispatchedBDPUFrames++;
    send(frame, "ifOut", portNum);
}

void Ieee8021dRelay::deliverBPDU(EtherFrame *frame)
{
    BPDU *bpdu = check_and_cast<BPDU *>(frame->decapsulate());

    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setSrc(frame->getSrc());
    controlInfo->setSwitchPort(frame->getArrivalGate()->getIndex());
    controlInfo->setDest(frame->getDest());

    bpdu->setControlInfo(controlInfo);

    delete frame;    // we have the BPDU packet, so delete the frame

    EV_INFO << "Sending BPDU frame " << bpdu << " to the STP/RSTP module" << endl;
    numDeliveredBDPUsToSTP++;
    send(bpdu, "stpOut");
}

Ieee8021dInterfaceData *Ieee8021dRelay::getPortInterfaceData(unsigned int portNum)
{
    if (isStpAware) {
        cGate *gate = getContainingNode(this)->gate("ethg$o", portNum);
        InterfaceEntry *gateIfEntry = ifTable->getInterfaceByNodeOutputGateId(gate->getId());
        Ieee8021dInterfaceData *portData = gateIfEntry ? gateIfEntry->ieee8021dData() : nullptr;

        if (!portData)
            throw cRuntimeError("Ieee8021dInterfaceData not found for port = %d", portNum);

        return portData;
    }
    return nullptr;
}

void Ieee8021dRelay::start()
{
    isOperational = true;

    ie = chooseInterface();
    if (ie)
        bridgeAddress = ie->getMacAddress(); // get the bridge's MAC address
    else
        throw cRuntimeError("No non-loopback interface found!");

    macTable->clearTable();
}

void Ieee8021dRelay::stop()
{
    isOperational = false;

    macTable->clearTable();
    ie = nullptr;
}

InterfaceEntry *Ieee8021dRelay::chooseInterface()
{
    // TODO: Currently, we assume that the first non-loopback interface is an Ethernet interface
    //       since relays work on EtherSwitches.
    //       NOTE that, we don't check if the returning interface is an Ethernet interface!
    IInterfaceTable *ift = check_and_cast<IInterfaceTable *>(getModuleByPath(par("interfaceTablePath")));

    for (int i = 0; i < ift->getNumInterfaces(); i++) {
        InterfaceEntry *current = ift->getInterface(i);
        if (!current->isLoopback())
            return current;
    }

    return nullptr;
}

void Ieee8021dRelay::finish()
{
    recordScalar("number of received BPDUs from STP module", numReceivedBPDUsFromSTP);
    recordScalar("number of received frames from network (including BPDUs)", numReceivedNetworkFrames);
    recordScalar("number of dropped frames (including BPDUs)", numDroppedFrames);
    recordScalar("number of delivered BPDUs to the STP module", numDeliveredBDPUsToSTP);
    recordScalar("number of dispatched BPDU frames to the network", numDispatchedBDPUFrames);
    recordScalar("number of dispatched non-BDPU frames to the network", numDispatchedNonBPDUFrames);
}

bool Ieee8021dRelay::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}

} // namespace inet

