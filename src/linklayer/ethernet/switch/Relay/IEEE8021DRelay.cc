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

#include "IEEE8021DRelay.h"
#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"
#include "IEEE8021DInterfaceData.h"

Define_Module(IEEE8021DRelay);

void IEEE8021DRelay::initialize(int stage)
{

    if (stage == 0)
    {
        // number of ports
        portCount = gate("ifOut", 0)->size();
        if (gate("ifIn", 0)->size() != portCount)
            error("the sizes of the ifIn[] and ifOut[] gate vectors must be the same");
    }
    else if (stage == 1)
    {
        NodeStatus * nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;

        cModule * tmpMacTable = getParentModule()->getSubmodule("macTable");
        macTable = check_and_cast<MACAddressTable *>(tmpMacTable);

        ifTable = InterfaceTableAccess().get();
        InterfaceEntry * ifEntry = ifTable->getInterface(0);
        bridgeAddress = ifEntry->getMacAddress();

        WATCH(bridgeAddress);
    }
}

void IEEE8021DRelay::handleMessage(cMessage * msg)
{
    if (!isOperational)
    {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }

    if (!msg->isSelfMessage())
    {
        // Messages from STP process
        if (strcmp(msg->getArrivalGate()->getName(), "STPGate$i") == 0)
        {
            BPDU * bpdu = check_and_cast<BPDU* >(msg);
            dispatchBPDU(bpdu);
        }
        // Messages from network
        else if (strcmp(msg->getArrivalGate()->getName(), "ifIn") == 0)
        {
            EthernetIIFrame * frame = check_and_cast<EthernetIIFrame*>(msg);
            handleAndDispatchFrame(frame);
        }
    }
}

void IEEE8021DRelay::broadcast(EthernetIIFrame * frame)
{
    unsigned int arrivalGate = frame->getArrivalGate()->getIndex();

    for (unsigned int i = 0; i < portCount; i++)
        if (i != arrivalGate && getPortInterfaceData(i)->isForwarding())
            dispatch(frame->dup(), i);

    delete frame;
}

void IEEE8021DRelay::handleAndDispatchFrame(EthernetIIFrame * frame)
{
    int arrivalGate = frame->getArrivalGate()->getIndex();

    // Broadcast address
    if (frame->getDest().isBroadcast())
        broadcast(frame);

    // BPDU Handling
    if (frame->getDest() == MACAddress::STP_MULTICAST_ADDRESS || frame->getDest() == bridgeAddress)
        deliverBPDU(frame); // Deliver to the STP/RSTP module
    else
    {
        int outGate = macTable->getPortForAddress(frame->getDest());
        learn(frame);
        // Not known -> broadcast
        if (outGate == -1)
            broadcast(frame);
        else
        {
            IEEE8021DInterfaceData * port = getPortInterfaceData(arrivalGate);
            if (port->isForwarding() && outGate != arrivalGate)
                dispatch(frame, outGate);
            else
                delete frame; // Drop this frame
        }
    }

}

void IEEE8021DRelay::dispatch(EthernetIIFrame * frame, unsigned int portNum)
{
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);

    // Padding
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);

    if (portNum >= portCount)
        return;

    send(frame, "ifOut", portNum);

    return;
}

void IEEE8021DRelay::learn(EthernetIIFrame * frame)
{
    int arrivalGate = frame->getArrivalGate()->getIndex();
    IEEE8021DInterfaceData * port = getPortInterfaceData(arrivalGate);

    if (port->isLearning())
        macTable->updateTableWithAddress(arrivalGate, frame->getSrc());
}

void IEEE8021DRelay::dispatchBPDU(BPDU * bpdu)
{
    Ieee802Ctrl * controlInfo = dynamic_cast<Ieee802Ctrl *>(bpdu->removeControlInfo());
    unsigned int port = controlInfo->getInterfaceId();
    MACAddress address = controlInfo->getDest();
    delete controlInfo;

    if (port >= portCount || port < 0)
        return;

    // TODO: use LLCFrame
    EthernetIIFrame * frame = new EthernetIIFrame(bpdu->getName());

    frame->setKind(bpdu->getKind());
    frame->setSrc(bridgeAddress);
    frame->setDest(address);
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    frame->setEtherType(-1);
    frame->encapsulate(bpdu);

    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);

    send(frame, "ifOut", port);
}

void IEEE8021DRelay::deliverBPDU(EthernetIIFrame * frame)
{
    BPDU * bpdu = check_and_cast<BPDU *>(frame->decapsulate());

    Ieee802Ctrl * controlInfo = new Ieee802Ctrl();
    controlInfo->setSrc(frame->getSrc());
    controlInfo->setInterfaceId(frame->getArrivalGate()->getIndex());
    controlInfo->setDest(frame->getDest());

    bpdu->setControlInfo(controlInfo);

    delete frame; // We have the BPDU packet, so delete the frame

    send(bpdu, "STPGate$o");
}

IEEE8021DInterfaceData * IEEE8021DRelay::getPortInterfaceData(unsigned int portNum)
{
    cGate * gate = this->getParentModule()->gate("ethg$o", portNum);
    InterfaceEntry * gateIfEntry = ifTable->getInterfaceByNodeOutputGateId(gate->getId());
    IEEE8021DInterfaceData * portData = gateIfEntry->ieee8021DData();

    if (!portData)
        error("IEEE8021DInterfaceData not found!");

    return portData;
}

void IEEE8021DRelay::start()
{
    macTable->clearTable();
    isOperational = true;
}

void IEEE8021DRelay::stop()
{
    macTable->clearTable();
    isOperational = false;
}

bool IEEE8021DRelay::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER)
        {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_LINK_LAYER)
        {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH)
        {
            stop();
        }
    }
    else
    {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}
