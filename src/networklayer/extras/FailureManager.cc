//
// Copyright (C) 2005 Vojtech Janota
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

#include "FailureManager.h"

Define_Module(FailureManager);

void FailureManager::initialize()
{
    // so far no initialization
}

void FailureManager::handleMessage(cMessage *msg)
{
    ASSERT(false);
}

cModule *FailureManager::getTargetNode(const char *target)
{
    cModule *mod = simulation.getModuleByPath(target);
    ASSERT(mod);
    return mod;
}

void FailureManager::processCommand(const cXMLElement& node)
{
    cModule *target = getTargetNode(node.getAttribute("target"));

    if (!strcmp(node.getTagName(), "shutdown"))
    {
        target->getDisplayString().setTagArg("i2", 0, "status/cross");
        if (!strcmp(target->getModuleType()->getName(), "RSVP_LSR"))
            replaceNode(target, "inet.nodes.mpls.RSVP_FAILED");
        else if (!strcmp(target->getModuleType()->getName(), "LDP_LSR"))
            replaceNode(target, "inet.nodes.mpls.LDP_FAILED");
        else
            ASSERT(false);
    }
    else if (!strcmp(node.getTagName(), "startup"))
    {
        target->getDisplayString().removeTag("i2");
        if (!strcmp(target->getModuleType()->getName(), "RSVP_FAILED"))
            replaceNode(target, "inet.nodes.mpls.RSVP_LSR");
        else if (!strcmp(target->getModuleType()->getName(), "LDP_FAILED"))
            replaceNode(target, "inet.nodes.mpls.LDP_LSR");
        else
            ASSERT(false);
    }
    else
        ASSERT(false);

}

void FailureManager::replaceNode(cModule *mod, const char *newNodeType)
{
    ASSERT(mod);

    cModuleType *nodeType = cModuleType::find(newNodeType);
    if (!nodeType)
        error("Cannot replace module `%s' with a module of type `%s': No such module type", mod->getFullPath().c_str(), newNodeType);

    cModule *nodeMod = nodeType->create(mod->getName(), simulation.getSystemModule());
    ASSERT(nodeMod);

    reconnectNode(mod, nodeMod);
    delete mod;

    nodeMod->buildInside();
    nodeMod->scheduleStart(simTime());
    nodeMod->callInitialize();
}

void FailureManager::reconnectNode(cModule *old, cModule *n)
{
    copyParams(old, n);
    n->finalizeParameters();
    n->setDisplayString(old->getDisplayString().str());

    reconnectAllGates(old, n);
}

void FailureManager::reconnectAllGates(cModule *old, cModule *n)
{
    std::vector<const char*> gateNames = old->getGateNames();
    for (std::vector<const char*>::const_iterator it=gateNames.begin(); it!=gateNames.end(); ++it)
    {
        const char* gateName = *it;
        if (old->isGateVector(gateName))
        {
            unsigned int size = old->gateSize(gateName);
            n->setGateSize(gateName, size);
            for (unsigned int i = 0; i < size; i++)
                reconnectGates(old, n, gateName, i);
        }
        else
        {
            reconnectGates(old, n, gateName);
        }
    }
}

void FailureManager::reconnectGates(cModule *old, cModule *n, const char *gateName, int gateIndex)
{
    cGate::Type gateType = old->gateType(gateName);
    if (gateType == cGate::INOUT)
    {
        std::string ingateName = (std::string(gateName)+"$i");
        std::string outgateName = (std::string(gateName)+"$o");
        reconnectGate(old->gate(ingateName.c_str(), gateIndex), n->gate(ingateName.c_str(), gateIndex));
        reconnectGate(old->gate(outgateName.c_str(), gateIndex), n->gate(outgateName.c_str(), gateIndex));
    }
    else
    {
        reconnectGate(old->gate(gateName, gateIndex), n->gate(gateName, gateIndex));
    }
}

void FailureManager::reconnectGate(cGate *oldGate, cGate *newGate)
{
    cGate::Type gateType = oldGate->getType();
    if (gateType == cGate::OUTPUT)
    {
        if (oldGate->isConnected())
        {
            cGate *to = oldGate->getNextGate();
            cChannel *ch = copyChannel(oldGate->getChannel());
            std::string displayString = oldGate->getChannel()->getDisplayString().str();
            oldGate->disconnect();
            newGate->connectTo(to, ch);
            ch->setDisplayString(displayString.c_str());
        }
    }
    else if (gateType == cGate::INPUT)
    {
        if (oldGate->isConnected())
        {
            cGate *from = oldGate->getPreviousGate();
            cChannel *ch = copyChannel(from->getChannel());
            std::string displayString = from->getChannel()->getDisplayString().str();
            from->disconnect();
            from->connectTo(newGate, ch);
            ch->setDisplayString(displayString.c_str());
        }
    }
    else
    {
        error("Unexpected gate type: %d", (int)gateType);
    }
}

cChannel *FailureManager::copyChannel(cChannel *channel)
{
    cChannel *copy = channel->getChannelType()->create(channel->getName());
    copyParams(channel, copy);
    return copy;
}

void FailureManager::copyParams(cComponent *from, cComponent *to)
{
    for (int i = 0; i < from->getNumParams(); i++)
        to->par(i) = from->par(i);
}

