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

#ifndef __INET_HTTPSERVERBASE_H
#define __INET_HTTPSERVERBASE_H

#include <string>
#include <vector>
#include "inet/applications/httptools/common/HttpNodeBase.h"

namespace inet {

namespace httptools {

// Event message kinds
#define MSGKIND_START_SESSION    0
#define MSGKIND_NEXT_MESSAGE     1
#define MSGKIND_SCRIPT_EVENT     2

/**
 * Web server base class.
 *
 * This module implements a flexible Web server. It is part of the HttpTools project
 * and should be used in conjunction with a number of browsing clients.
 *
 * The server base class cannot be instantiated directly in a simulation. Use rather
 * the HttpServer for INET TCP/IP applications or HttpServerDirect for direct message passing.
 * See those classes for details. See the INET documentation for details on the StandardHost
 * and the TCP/IP simulation.
 *
 * @see HttpServer
 * @see HttpServerDirect
 * @see DirectHost.
 *
 * @author  Kristjan V. Jonsson
 */
class INET_API HttpServerBase : public HttpNodeBase
{
  protected:
    /**
     * Describes a HTML page
     */
    struct HtmlPageData
    {
        long size;
        std::string body;
    };

    std::string hostName;    // the server name, e.g. www.example.com.
    int port;    // the listening port of the server
    bool scriptedMode;    // set to true if a scripted site definition is used
    std::map<std::string, HtmlPageData> htmlPages;    // A map of html pages, keyed by a resource URL. Used in scripted mode
    std::map<std::string, unsigned int> resources;    // a map of resource, keyed by a resource URL. Used in scripted mode
    simtime_t activationTime;    // the activation time of the server -- initial startup delay

    // Basic statistics
    long htmlDocsServed;
    long imgResourcesServed;
    long textResourcesServed;
    long badRequests;

    rdObject *rdReplyDelay;    ///< The processing delay of the server.
    rdObject *rdHtmlPageSize;    ///< The HTML page size distribution for the site.
    rdObject *rdTextResourceSize;    ///< The text resource size distribution for the site.
    rdObject *rdImageResourceSize;    ///< The image resource size distribution for the site.
    rdObject *rdNumResources;    ///< Number of resources per HTML page.
    rdObject *rdTextImageResourceRatio;    ///< The ratio of text resources to images referenced in HTML pages.
    rdObject *rdErrorMsgSize;    ///< The size of error messages.

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void finish();
    virtual void handleMessage(cMessage *msg) = 0;

    void updateDisplay();
    HttpReplyMessage *generateDocument(HttpRequestMessage *request, const char *resource, int size = 0);
    HttpReplyMessage *generateResourceMessage(HttpRequestMessage *request, std::string resource, HttpContentType category);
    HttpReplyMessage *handleGetRequest(HttpRequestMessage *request, std::string resource);
    HttpReplyMessage *generateErrorReply(HttpRequestMessage *request, int code);
    virtual std::string generateBody();
    cPacket *handleReceivedMessage(cMessage *msg);
    void registerWithController();
    void readSiteDefinition(std::string file);
    std::string readHtmlBodyFile(std::string file, std::string path);

  public:
    HttpServerBase();
    ~HttpServerBase();

    /*
     * Return the name of the server
     */
    const std::string& getHostName() { return hostName; }
};

} // namespace httptools

} // namespace inet

#endif // ifndef __INET_HTTPSERVERBASE_H

