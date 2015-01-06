//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
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

#include "inet/applications/httptools/server/HttpServerDirect.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace httptools {

Define_Module(HttpServerDirect);

void HttpServerDirect::initialize(int stage)
{
    HttpServerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        EV_DEBUG << "Initializing direct server component\n";

        // Set the link speed
        linkSpeed = par("linkSpeed");
    }
}

void HttpServerDirect::finish()
{
    HttpServerBase::finish();
}

void HttpServerDirect::handleMessage(cMessage *msg)
{
    EV_DEBUG << "Handling received message " << msg->getName() << endl;
    if (msg->isSelfMessage()) {
        // Self messages are not used at the present
    }
    else {
        HttpNodeBase *senderModule = dynamic_cast<HttpNodeBase *>(msg->getSenderModule());
        if (senderModule == nullptr) {
            EV_ERROR << "Unspecified sender module in received message " << msg->getName() << endl;
            delete msg;
            return;
        }
        cModule *senderHost = getContainingNode(senderModule);
        EV_DEBUG << "Sender is " << senderModule->getFullName()
                 << " in host " << senderHost->getFullName() << endl;
        cPacket *reply = handleReceivedMessage(msg);
        // Echo back to the requester
        if (reply != nullptr)
            sendDirectToModule(senderModule, reply, 0.0, rdReplyDelay);
        delete msg;
    }
    updateDisplay();
}

} // namespace httptools

} // namespace inet

