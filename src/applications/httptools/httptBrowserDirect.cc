
// ***************************************************************************
//
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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
// ***************************************************************************

#include "httptBrowserDirect.h"

Define_Module(httptBrowserDirect);

void httptBrowserDirect::initialize(int stage)
{
	httptBrowserBase::initialize(stage);
	EV_DEBUG << "Initializing HTTP direct browser component\n";

	// linkSpeed is used to model transmission delay. By default, 1 Mbit/s.
	linkSpeed = par("linkSpeed");
	if ( linkSpeed == 0 ) linkSpeed = 1024*1024;
	else linkSpeed *= 1024*1024; // @todo: use the units convention for the ini file rather than multiplying to MBit/s
}

void httptBrowserDirect::finish()
{
	httptBrowserBase::finish();
}

void httptBrowserDirect::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
       	handleSelfMessages(msg);
	else
		handleDataMessage(msg);
}

void httptBrowserDirect::sendRequestToServer( BROWSE_EVENT_ENTRY be )
{
	sendDirectToModule(be.serverModule,generatePageRequest(be.wwwhost,be.resourceName),0.0,rdProcessingDelay);
}

void httptBrowserDirect::sendRequestToServer( httptRequestMessage *request )
{
	httptNodeBase *serverModule = dynamic_cast<httptNodeBase*>(controller->getServerModule(request->targetUrl()));
	if ( serverModule == NULL )
	{
		EV_ERROR << "Failed to get server module for " << request->targetUrl() << endl;
	}
	else
	{
		EV_DEBUG << "Sending request to " << serverModule->getWWW() << endl;
		sendDirectToModule(serverModule,request,0.0,rdProcessingDelay);
	}
}

void httptBrowserDirect::sendRequestToRandomServer()
{
	httptNodeBase *serverModule = dynamic_cast<httptNodeBase*>(controller->getAnyServerModule());
	if ( serverModule == NULL )
	{
		EV_ERROR << "Failed to get a random server module" << endl;
	}
	else
	{
		EV_DEBUG << "Sending request randomly to " << serverModule->getWWW() << endl;
		sendDirectToModule(serverModule,generateRandomPageRequest(serverModule->getWWW()),0.0,rdProcessingDelay);
	}
}

void httptBrowserDirect::sendRequestsToServer( string www, MESSAGE_QUEUE_TYPE queue )
{
	cMessage *msg;
	httptNodeBase *serverModule = dynamic_cast<httptNodeBase*>(controller->getServerModule(www.c_str()));
	if ( serverModule == NULL )
		EV_ERROR << "Failed to get server module " << www << endl;
	else
	{
		while( queue.size()!=0 )
		{
			msg = queue.back();
			queue.pop_back();
			sendDirectToModule(serverModule,msg,0.0,rdProcessingDelay);
		}
	}
}

