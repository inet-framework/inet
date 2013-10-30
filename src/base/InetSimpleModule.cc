//
// Copyright (C) 2004, 2008 Andras Varga
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

#include "InetSimpleModule.h"

#define TRY(code, msgprefix) try {code;} catch(cRuntimeError& e) {e.prependMessage(msgprefix);throw;}

int InetSimpleModule::sendSync(cMessage *msg, const char *gateName, int gateIndex)
{
    cGate *outgate;
    TRY(outgate = gate(gateName,gateIndex), "sendSync()");
    return sendSync(msg, outgate);
}

int InetSimpleModule::sendSync(cMessage *msg, int gateId)
{
    cGate *outgate;
    TRY(outgate = gate(gateId), "sendSync()");
    return sendSync(msg, outgate);
}

int InetSimpleModule::sendSync(cMessage *msg, cGate *outgate)
{
    // error checking:
    if (outgate==NULL)
       throw cRuntimeError("sendSync(): gate pointer is NULL");
    if (outgate->getType()==cGate::INPUT)
       throw cRuntimeError("sendSync(): cannot send via an input gate (`%s')", outgate->getFullName());
    if (!outgate->getNextGate())  // NOTE: without this error check, msg would become self-message
       throw cRuntimeError("sendSync(): gate `%s' not connected", outgate->getFullName());
    if (msg==NULL)
        throw cRuntimeError("sendSync(): message pointer is NULL");
    if (msg->getOwner()!=this)
    {
        if (this!=simulation.getContextModule() && simulation.getContextModule()!=NULL)
            throw cRuntimeError("sendSync() of module (%s)%s called in the context of "
                                "module (%s)%s: method called from the latter module "
                                "lacks Enter_Method() or Enter_Method_Silent()? "
                                "Also, if message to be sent is passed from that module, "
                                "you'll need to call take(msg) after Enter_Method() as well",
                                getClassName(), getFullPath().c_str(),
                                simulation.getContextModule()->getClassName(),
                                simulation.getContextModule()->getFullPath().c_str());
        else if (msg->getOwner()==&simulation.msgQueue && msg->isSelfMessage() && msg->getArrivalModuleId()==getId())
            throw cRuntimeError("sendSync(): cannot send message (%s)%s, it is "
                                "currently scheduled as a self-message for this module",
                                msg->getClassName(), msg->getName());
        else if (msg->getOwner()==&simulation.msgQueue && msg->isSelfMessage())
            throw cRuntimeError("sendSync(): cannot send message (%s)%s, it is "
                                "currently scheduled as a self-message for ANOTHER module",
                                msg->getClassName(), msg->getName());
        else if (msg->getOwner()==&simulation.msgQueue)
            throw cRuntimeError("sendSync(): cannot send message (%s)%s, it is "
                                "currently in scheduled-events, being underway between two modules",
                                msg->getClassName(), msg->getName());
        else
            throw cRuntimeError("sendSync(): cannot send message (%s)%s, "
                                "it is currently contained/owned by (%s)%s",
                                msg->getClassName(), msg->getName(), msg->getOwner()->getClassName(),
                                msg->getOwner()->getFullPath().c_str());
    }
    cGate *destGate = outgate->getPathEndGate();
    InetSimpleModule *destModule = dynamic_cast<InetSimpleModule*>(destGate->getOwnerModule());
    if (!destModule)
        throw cRuntimeError("sendSync(): cannot send to module (%s)%s, it is not an InetSimpleModule",
                            destModule->getNedTypeName(), destModule->getFullPath().c_str());
    if (destModule->usesActivity())
        throw cRuntimeError("sendSync(): cannot send to module (%s)%s, it uses activity() not handleMessage()",
                            destModule->getNedTypeName(), destModule->getFullPath().c_str());


    // set message parameters and send it
    msg->setSentFrom(this, outgate->getId(), simTime());
    if (msg->isPacket())
        ((cPacket *)msg)->setDuration(0);
    msg->setArrival(destModule, destGate->getId());
    msg->setArrivalTime(simTime());

    destModule->pubHandleMessage(msg);
    return 0;
}

void InetSimpleModule::pubHandleMessage(cMessage *msg)
{
    Enter_Method("handleMessage(%s)", msg->getName());
    take(msg);
    EV << "Entering " << getClassName() << "::handleMessage() " << getFullPath() << "\n";
    handleMessage(msg);
    EV << "Leaving " << getClassName() << "::handleMessage() " << getFullPath() << "\n";
}

