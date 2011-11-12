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


#ifndef __INET_HTTPNODEBASE_H
#define __INET_HTTPNODEBASE_H

#include <string>
#include <map>
#include <queue>
#include <iostream>
#include <fstream>

#include "INETDefs.h"

#include "HttpController.h"
#include "HttpMessages_m.h"
#include "HttpRandom.h"
#include "HttpUtils.h"
#include "HttpLogdefs.h"

#define HTTPT_REQUEST_MESSAGE           10000
#define HTTPT_DELAYED_REQUEST_MESSAGE   10001
#define HTTPT_RESPONSE_MESSAGE          10010
#define HTTPT_DELAYED_RESPONSE_MESSAGE  10011

enum LOG_FORMAT {lf_short, lf_long};

/**
 * The base class for browser and server nodes.
 *
 * See the derived classes HttpBrowserBase and HttpServerBase for details
 *
 * @see HttpBrowserBase
 * @see HttpServerBase
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 * @version 1.0
 */
class HttpNodeBase : public cSimpleModule
{
    protected:
        /** Log level 2: Debug, 1: Info; 0: Errors and warnings only */
        int ll;

        /** The link speed in bits per second. Only needed for direct message passing transmission delay calculations. */
        unsigned long linkSpeed;

        /** The http protocol. http/1.0: 10 ; http/1.1: 11 */
        int httpProtocol;

        /** The log file name for message generation events */
        std::string logFileName;
        /** Enable/disable of logging message generation events to file */
        bool enableLogging;
        /** The format used to log message events to the log file (if enabled) */
        LOG_FORMAT outputFormat;
        /** Enable/disable logging of message events to the console */
        bool m_bDisplayMessage;
        /** Enable/disable of logging message contents (body) to the console. Only if m_bDisplayMessage is set */
        bool m_bDisplayResponseContent;

    public:
        HttpNodeBase();

    protected:
        /** @name Direct message passing utilities */
        //@{
        /**
         * Send a single message direct to the specified module.
         * Transmission delay is automatically calculated from the size of the message. In addition, a constant delay
         * a random delay object may be specified. Those delays add to the total used to submit the message to the
         * OMNeT++ direct message passing mechanism.
         */
        void sendDirectToModule(HttpNodeBase *receiver, cPacket *packet, simtime_t constdelay = 0.0, rdObject *rd = NULL);
        /** Calculate the transmission delay for the packet */
        double transmissionDelay(cPacket *packet);
        //@}

    protected:
        /** @name Methods for logging and formatting messages */
        //@{
        /** Log a request message to file and optionally display in console */
        void logRequest(const HttpRequestMessage* httpRequest);
        /** Log a response message to file and optionally display in console */
        void logResponse(const HttpReplyMessage* httpResponse);
        /** Used by logRequest and logResponse to write the formatted message to file */
        void logEntry(std::string line);
        /** Format a request message in compact semicolon-delimited format */
        std::string formatHttpRequestShort(const HttpRequestMessage* httpRequest);
        /** Format a response message in compact semicolon-delimited format */
        std::string formatHttpResponseShort(const HttpReplyMessage* httpResponse);
        /** Format a request message in a more human-readable format */
        std::string formatHttpRequestLong(const HttpRequestMessage* httpRequest);
        /** Format a response message in a more human-readable format */
        std::string formatHttpResponseLong(const HttpReplyMessage* httpResponse);
        //@}
};

#endif


