//
// Copyright (C) 2011 Juan Luis Garrote Molinero
// Copyright (C) 2013 Zsolt Prontvai
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

#include "RelayRSTP.h"

#include "Ethernet.h"
#include "RSTPAccess.h"
#include "MACAddressTableAccess.h"
#include "ModuleAccess.h"
#include "NodeOperations.h"
#include "NodeStatus.h"

Define_Module(RelayRSTP);

void RelayRSTP::initialize(int stage)
{
    if (stage == 1)
    {
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
    }
    else if(stage == 2)  // "auto" MAC addresses assignment takes place in stage 0. rstpModule gets address in stage 1.
    {
        //Obtaining AddressTable, rstp modules pointers
        AddressTable = MACAddressTableAccess().get();
        rstpModule = RSTPAccess().get();

        //Gets bridge MAC address from rstpModule
        address=rstpModule->getAddress();

        //Gets parameter values
        verbose=(bool) par("verbose");
        if(verbose==true)
            ev<<"Bridge MAC address "<<address.str()<<endl;
    }
}

void RelayRSTP::handleMessage(cMessage *msg)
{
    if (!isOperational)
    {
        EV << "Message '" << msg << "' arrived when module status is down, dropped it\n";
        delete msg;
        return;
    }
    ev << "New message"<<endl;

    //rstp module update.
    rstpModule=RSTPAccess().get();

    //Sends to correct handler after verification
    if (msg->arrivedOn("RSTPGate$i"))
    { //Outgoing Frame. Ieee802Ctrl encapsulates the frame.
        ev<<"Outgoing Frame";
        handleFrameFromRSTP(check_and_cast<BPDUieee8021D *> (msg));
    }
    else if (dynamic_cast<EtherFrame *>(msg) != NULL && check_and_cast<EtherFrame *> (msg)->getDest()==MACAddress::STP_MULTICAST_ADDRESS) //TODO: Find a better condition
    {
        ev<<"BPDU";
        int ArrivalRole=rstpModule->getPortRole(msg->getArrivalGate()->getIndex());
        if(ArrivalRole!=DISABLED)
            handleBPDUFrame(check_and_cast<EtherFrame *> (msg));
        delete msg;
    }
    else if(dynamic_cast<EtherFrame *>(msg) != NULL)
    {
        ev<<"Data Frame";
        int ArrivalState=rstpModule->getPortState(msg->getArrivalGate()->getIndex());
        if((ArrivalState==FORWARDING)||(ArrivalState==LEARNING))
        {
            ev<<"Handling Ethernet Frame"<<endl;
            handleEtherFrame(check_and_cast<EtherFrame *> (msg));
        }
        else
        {
            delete msg;
            ev<<"Discarding Frame"<<endl;
        }
    }
    else
    {
        error("Not supported frame type");
    }
}

void RelayRSTP::handleBPDUFrame(EtherFrame *frame)
{
    if((frame->getDest()==address)||(frame->getDest()==MACAddress::STP_MULTICAST_ADDRESS))
    {
        BPDUieee8021D *bpduFrame = check_and_cast<BPDUieee8021D *>(PK(frame)->decapsulate());
        Ieee802Ctrl * etherctrl= new Ieee802Ctrl();
        etherctrl->setInterfaceId(frame->getArrivalGate()->getIndex());
        etherctrl->setDest(frame->getDest());
        etherctrl->setSrc(frame->getSrc());
        bpduFrame->setControlInfo(etherctrl);
        send(bpduFrame,"RSTPGate$o"); //And sends the frame to the RSTP module.
    }
    else
    {
        ev<<"Wrong formated BPDU";
        delete frame;
    }
}

void RelayRSTP::handleFrameFromRSTP(BPDUieee8021D *frame)
{
    Ieee802Ctrl * etherctrl=check_and_cast<Ieee802Ctrl *>(frame->removeControlInfo());
    EtherFrame *eth2Frame = new EthernetIIFrame(frame->getName());;
    eth2Frame->setSrc(etherctrl->getSrc());  // if blank, will be filled in by MAC
    eth2Frame->setDest(etherctrl->getDest());
    eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    eth2Frame->encapsulate(frame);
    int sendBy=etherctrl->getInterfaceId();
    delete etherctrl;

    send(eth2Frame,"GatesOut",sendBy);
}

void RelayRSTP::handleEtherFrame(EtherFrame *frame)
{
    int ArrivalState=rstpModule->getPortState(frame->getArrivalGate()->getIndex());
    int arrival=frame->getArrivalGate()->getIndex();
    EtherFrame * EthIITemp= frame;
    if(verbose==true)
    {
        AddressTable->printState();  //Shows AddressTable info.
    }

    //Learning in case of FORWARDING or LEARNING state.
    if((ArrivalState==FORWARDING)|| (ArrivalState==LEARNING))
    {
        if(EthIITemp->getSrc()!= MACAddress::UNSPECIFIED_ADDRESS )
        {
            AddressTable->updateTableWithAddress(arrival,EthIITemp->getSrc()); //Registers source at arrival gate.
        }
    }
    //Processing in case of FORWARDING state
    if(ArrivalState==FORWARDING)
    {
        int outputPort;
        if (EthIITemp->getDest().isBroadcast())
        {
            broadcastMsg(frame);
        }
        else if((outputPort=AddressTable->getPortForAddress(EthIITemp->getDest()))!=-1)
        {
            relayMsg(frame,outputPort);
        }
        else
        {
            broadcastMsg(frame);
        }
    }
    else
    {
        delete frame;
    }
}

void RelayRSTP::relayMsg(cMessage * msg, int outputPort)
{
    int arrival=msg->getArrivalGate()->getIndex();
    int outputState=rstpModule->getPortState(outputPort);
    if((arrival!=outputPort)&&(outputState==FORWARDING))
    {
        if(verbose==true)
            ev << "Sending frame to port " << outputPort << endl;
        send(msg,"GatesOut",outputPort);
    }
}

void RelayRSTP::broadcastMsg(cMessage * msg)
{
    int arrival=msg->getArrivalGate()->getIndex();
    int gates=gateSize("GatesOut");
    for(int i=0;i<gates;i++)
    {
        int outputState=rstpModule->getPortState(i);
        if((arrival!=i)&&(outputState==FORWARDING))
        {
            if(verbose==true)
                ev << "Sending frame to port " << i << endl;
            cMessage * msg2=msg->dup();
            send(msg2,"GatesOut",i);
        }
    }
    delete msg;
}
void RelayRSTP::start()
{
    AddressTable->clearTable();
    isOperational = true;
}

void RelayRSTP::stop()
{
    AddressTable->clearTable();
    isOperational = false;
}

bool RelayRSTP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    if (dynamic_cast<NodeStartOperation *>(operation))
    {
        if (stage == NodeStartOperation::STAGE_LINK_LAYER) {
            start();
        }
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation))
    {
        if (stage == NodeShutdownOperation::STAGE_LINK_LAYER) {
            stop();
        }
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation))
    {
        if (stage == NodeCrashOperation::STAGE_CRASH) {
            stop();
        }
    }
    else
    {
        throw cRuntimeError("Unsupported operation '%s'", operation->getClassName());
    }

    return true;
}
