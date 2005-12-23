//
// Copyright (C) 2005 Vojtech Janota
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    cModule *mod = simulation.moduleByPath(target);
    ASSERT(mod);
    return mod;
}

void FailureManager::processCommand(const cXMLElement& node)
{
    cModule *target = getTargetNode(node.getAttribute("target"));

    if (!strcmp(node.getTagName(), "shutdown"))
    {
        target->displayString().setTagArg("i2",0,"status/cross");
        if(!strcmp(target->moduleType()->name(), "RSVP_LSR"))
            replaceNode(target, "RSVP_FAILED");
        else if(!strcmp(target->moduleType()->name(), "LDP_LSR"))
            replaceNode(target, "LDP_FAILED");
        else if(!strcmp(target->moduleType()->name(), "QuaggaRouter"))
            replaceNode(target, "FailedRouter");
        else
            ASSERT(false);
    }
    else if(!strcmp(node.getTagName(), "startup"))
    {
        target->displayString().removeTag("i2");
        if(!strcmp(target->moduleType()->name(), "RSVP_FAILED"))
            replaceNode(target, "RSVP_LSR");
        else if(!strcmp(target->moduleType()->name(), "LDP_FAILED"))
            replaceNode(target, "LDP_LSR");
        else if(!strcmp(target->moduleType()->name(), "FailedRouter"))
            replaceNode(target, "QuaggaRouter");
        else
            ASSERT(false);
    }
    else
        ASSERT(false);

}

void FailureManager::replaceNode(cModule *mod, const char *newNodeType)
{
    cModuleType *nodeType = findModuleType(newNodeType);
    ASSERT(nodeType);
    cModule *nodeMod = nodeType->create(mod->name(), simulation.systemModule());
    ASSERT(nodeMod);

    reconnectNode(mod, nodeMod);
    delete mod;

    nodeMod->buildInside();
    nodeMod->scheduleStart(simTime());
    nodeMod->callInitialize();
}

void FailureManager::reconnectNode(cModule *old, cModule *n)
{
    for(int i = 0; i < old->params(); i++)
        n->par(i) = old->par(i);

    n->setDisplayString(old->displayString().getString());

    // FIXME should examine which gates the "old" module has, and reconnect all of them
    // automatically (ie eliminate hardcoded gate names from here)
    reconnect(old, n, "in", "out");
    //reconnect(old, n, "ethIn", "ethOut");

}

void FailureManager::reconnect(cModule *old, cModule *n, const char *ins, const char *outs)
{
    unsigned int insize = old->gateSize(ins);
    unsigned int outsize = old->gateSize(outs);

    n->setGateSize(ins, insize);
    n->setGateSize(outs, outsize);

    for(unsigned int i = 0; i < outsize; i++)
    {
        cGate *out = old->gate(outs, i);
        if(out->isConnected())
        {
            cGate *to = out->toGate();
            cChannel *ch = check_and_cast<cChannel*>(out->channel()->dup());
            out->disconnect();
            n->gate(outs, i)->connectTo(to, ch);
        }
    }

    for (unsigned int i = 0; i < insize; i++)
    {
        cGate *in = old->gate(ins, i);
        if (in->isConnected())
        {
            cGate *from = in->fromGate();
            cChannel *ch = check_and_cast<cChannel*>(from->channel()->dup());
            from->disconnect();
            from->connectTo(n->gate(ins, i), ch);
        }
    }
}

