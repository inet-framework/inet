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

void FailureManager::processCommand(const cXMLElement& node)
{
	if(!strcmp(node.getTagName(), "shutdown"))
	{
		replaceNode(node.getAttribute("target"), "FAILED_LSR");
	}
	else if(!strcmp(node.getTagName(), "startup"))
	{
		replaceNode(node.getAttribute("target"), "RSVP_LSR");
	}
	else
	{
		ASSERT(false);
	}
	
}

void FailureManager::replaceNode(const char *target, const char *newNodeType)
{
	cModule *mod = simulation.moduleByPath(target);
	ASSERT(mod);
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

	unsigned int insize = old->gateSize("in");
	unsigned int outsize = old->gateSize("out");

	n->setGateSize("in", insize);
	n->setGateSize("out", outsize);

	for(unsigned int i = 0; i < outsize; i++)
	{
		cGate *out = old->gate("out", i);
		if(out->isConnected())
		{
			cGate *to = out->toGate();
			cChannel *ch = check_and_cast<cChannel*>(out->channel()->dup());
			out->disconnect();
			n->gate("out", i)->connectTo(to, ch);
		}
	}

	for(unsigned int i = 0; i < insize; i++)
	{
		cGate *in = old->gate("in", i);
		if(in->isConnected())
		{
			cGate *from = in->fromGate();
			cChannel *ch = check_and_cast<cChannel*>(from->channel()->dup());
			from->disconnect();
			from->connectTo(n->gate("in", i), ch);
		}
	}
}

