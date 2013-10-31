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

        ifTable = check_and_cast<IInterfaceTable*>(this->getParentModule()->getSubmodule("interfaceTable"));
        InterfaceEntry * ifEntry = ifTable->getInterface(0);
        bridgeAddress = ifEntry->getMacAddress();

        WATCH(bridgeAddress);
    }
}

void IEEE8021DRelay::handleMessage(cMessage * msg)
{
    if (!isOperational)
    {
        EV_ERROR << "Message '" << msg << "' arrived when module status is down, dropped it." << endl;
        delete msg;
        return;
    }

    if (!msg->isSelfMessage())
    {
        // Messages from STP process
        if (strcmp(msg->getArrivalGate()->getName(), "STPGate$i") == 0)
        {
            EV_INFO << "Received " << msg << " from STP/RSTP module." << endl;
            BPDU * bpdu = check_and_cast<BPDU* >(msg);
            dispatchBPDU(bpdu);
        }
        // Messages from network
        else if (strcmp(msg->getArrivalGate()->getName(), "ifIn") == 0)
        {
            EV_INFO << "Received " << msg << " from network." << endl;
            EtherFrame * frame = check_and_cast<EtherFrame*>(msg);
            handleAndDispatchFrame(frame);
        }
    }
    else
        delete msg;

}

void IEEE8021DRelay::broadcast(EtherFrame * frame)
{
    EV_DETAIL << "Broadcast frame " << frame << endl;

    unsigned int arrivalGate = frame->getArrivalGate()->getIndex();

    for (unsigned int i = 0; i < portCount; i++)
        if (i != arrivalGate && getPortInterfaceData(i)->isForwarding())
            dispatch(frame->dup(), i);

    delete frame;
}

void IEEE8021DRelay::handleAndDispatchFrame(EtherFrame * frame)
{
    int arrivalGate = frame->getArrivalGate()->getIndex();
    IEEE8021DInterfaceData * port = getPortInterfaceData(arrivalGate);
    // Broadcast address
    if (frame->getDest().isBroadcast())
    {
        broadcast(frame);
        return;
    }
    // BPDU Handling
    if ((frame->getDest() == MACAddress::STP_MULTICAST_ADDRESS || frame->getDest() == bridgeAddress) && port->getRole() != IEEE8021DInterfaceData::DISABLED)
    {
        EV_DETAIL << "Deliver BPDU to the STP/RSTP module" << endl;
        deliverBPDU(frame); // Deliver to the STP/RSTP module
    }
    else
    {
        learn(frame);
        int outGate = macTable->getPortForAddress(frame->getDest());
        // Not known -> broadcast
        if (outGate == -1)
        {
            EV_DETAIL << "Destination address = " << frame->getDest() << " unknown, broadcasting frame " << frame << endl;
            broadcast(frame);
        }
        else
        {
            if (port->isForwarding())
            {
                if (outGate != arrivalGate)
                    dispatch(frame, outGate);
                else
                {
                    EV_DETAIL << "Output port is same as input port, " << frame->getFullName() << " destination = " << frame->getDest() << ", discarding frame " << frame << endl;
                    delete frame;
                }
            }
            else
            {
                EV_DETAIL << "The input port = " << arrivalGate << " is not in state forwarding, discarding frame " << frame->getFullName() << endl;
                delete frame;
            }
        }
    }
}

void IEEE8021DRelay::dispatch(EtherFrame * frame, unsigned int portNum)
{
    IEEE8021DInterfaceData * port = getPortInterfaceData(portNum);

    if (portNum >= portCount)
        return;

    EV_INFO << "Sending " << frame << " with destination = " << frame->getDest() << ", port = " << portNum << endl;

    if (port->isForwarding())
        send(frame, "ifOut", portNum);

    return;
}

void IEEE8021DRelay::learn(EtherFrame * frame)
{
    int arrivalGate = frame->getArrivalGate()->getIndex();
    IEEE8021DInterfaceData * port = getPortInterfaceData(arrivalGate);

    if (port->isLearning())
        macTable->updateTableWithAddress(arrivalGate, frame->getSrc());
}

void IEEE8021DRelay::dispatchBPDU(BPDU * bpdu)
{
    Ieee802Ctrl * controlInfo = dynamic_cast<Ieee802Ctrl *>(bpdu->removeControlInfo());
    unsigned int portNum = controlInfo->getInterfaceId();
    MACAddress address = controlInfo->getDest();
    delete controlInfo;

    if (portNum >= portCount || portNum < 0)
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

    EV_INFO << "Sending BPDU frame " << frame << " with destination = " << frame->getDest() << ", port = " << portNum << endl;

    send(frame, "ifOut", portNum);
}

void IEEE8021DRelay::deliverBPDU(EtherFrame * frame)
{
    BPDU * bpdu = check_and_cast<BPDU *>(frame->decapsulate());

    Ieee802Ctrl * controlInfo = new Ieee802Ctrl();
    controlInfo->setSrc(frame->getSrc());
    controlInfo->setInterfaceId(frame->getArrivalGate()->getIndex());
    controlInfo->setDest(frame->getDest());

    bpdu->setControlInfo(controlInfo);

    delete frame; // We have the BPDU packet, so delete the frame

    EV_INFO << "Sending BPDU frame " << bpdu << " to the STP/RSTP module" << endl;
    send(bpdu, "STPGate$o");
}

IEEE8021DInterfaceData * IEEE8021DRelay::getPortInterfaceData(unsigned int portNum)
{
    cGate * gate = this->getParentModule()->gate("ethg$o", portNum);
    InterfaceEntry * gateIfEntry = ifTable->getInterfaceByNodeOutputGateId(gate->getId());
    IEEE8021DInterfaceData * portData = gateIfEntry->ieee8021DData();

    if (!portData)
        throw cRuntimeError("IEEE8021DInterfaceData not found for port = %d",portNum);

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
