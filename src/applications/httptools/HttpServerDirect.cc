// ***************************************************************************
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
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

#include "HttpServerDirect.h"

Define_Module(HttpServerDirect);

void HttpServerDirect::initialize()
{
    HttpServerBase::initialize();

    EV_DEBUG << "Initializing direct server component\n";

    // Set the link speed
    linkSpeed = par("linkSpeed");
}

void HttpServerDirect::finish()
{
    HttpServerBase::finish();
}

void HttpServerDirect::handleMessage(cMessage *msg)
{
    EV_DEBUG << "Handling received message " << msg->getName() << endl;
    if (msg->isSelfMessage())
    {
        // Self messages are not used at the present
    }
    else
    {
        HttpNodeBase *senderModule = dynamic_cast<HttpNodeBase*>(msg->getSenderModule());
        if (senderModule == NULL)
        {
            EV_ERROR << "Unspecified sender module in received message " << msg->getName() << endl;
            delete msg;
        }

        EV_DEBUG << "Sender is " << senderModule->getFullName()
                 << " in host " << senderModule->getParentModule()->getFullName() << endl;
        cPacket* reply = handleReceivedMessage(msg);
        // Echo back to the requester
        if (reply!=NULL)
            sendDirectToModule(senderModule, reply, 0.0, rdReplyDelay);
        delete msg;
    }
    updateDisplay();
}



