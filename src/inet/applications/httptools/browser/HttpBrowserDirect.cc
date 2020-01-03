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

#include "inet/applications/httptools/browser/HttpBrowserDirect.h"
#include "inet/applications/httptools/server/HttpServerBase.h"

namespace inet {

namespace httptools {

Define_Module(HttpBrowserDirect);

void HttpBrowserDirect::initialize(int stage)
{
    EV_DEBUG << "Initializing HTTP direct browser component - stage " << stage << endl;
    HttpBrowserBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // linkSpeed is used to model transmission delay.
        linkSpeed = par("linkSpeed");
    }
}

void HttpBrowserDirect::finish()
{
    HttpBrowserBase::finish();
}

void HttpBrowserDirect::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleSelfMessages(msg);
    else {
        const auto& appmsg = check_and_cast<Packet *>(msg)->peekAtFront<HttpReplyMessage>();
        handleDataMessage(appmsg);
        delete msg;
    }
}

void HttpBrowserDirect::sendRequestToServer(BrowseEvent be)
{
    sendDirectToModule(be.serverModule, generatePageRequest(be.wwwhost, be.resourceName), 0.0, rdProcessingDelay);
}

void HttpBrowserDirect::sendRequestToServer(Packet *pk)
{
    const auto& request = pk->peekAtFront<HttpRequestMessage>();
    HttpServerBase *serverModule = dynamic_cast<HttpServerBase *>(controller->getServerModule(request->getTargetUrl()));
    if (serverModule == nullptr) {
        EV_ERROR << "Failed to get server module for " << request->getTargetUrl() << endl;
        delete pk;
    }
    else {
        EV_DEBUG << "Sending request to " << serverModule->getHostName() << endl;
        sendDirectToModule(serverModule, pk, 0.0, rdProcessingDelay);
    }
}

void HttpBrowserDirect::sendRequestToRandomServer()
{
    HttpServerBase *serverModule = dynamic_cast<HttpServerBase *>(controller->getAnyServerModule());
    if (serverModule == nullptr) {
        EV_ERROR << "Failed to get a random server module" << endl;
    }
    else {
        EV_DEBUG << "Sending request randomly to " << serverModule->getHostName() << endl;
        sendDirectToModule(serverModule, generateRandomPageRequest(serverModule->getHostName()), 0.0, rdProcessingDelay);
    }
}

void HttpBrowserDirect::sendRequestsToServer(std::string www, HttpRequestQueue queue)
{
    HttpNodeBase *serverModule = dynamic_cast<HttpNodeBase *>(controller->getServerModule(www.c_str()));
    if (serverModule == nullptr) {
        EV_ERROR << "Failed to get server module " << www << endl;
        while (!queue.empty()) {
            Packet *msg = queue.back();
            queue.pop_back();
            delete msg;
        }
    }
    else {
        while (!queue.empty()) {
            Packet *msg = queue.back();
            queue.pop_back();
            sendDirectToModule(serverModule, msg, 0.0, rdProcessingDelay);
        }
    }
}

} // namespace httptools

} // namespace inet

