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


#ifndef __INET_HTTPBROWSERDIRECT_H
#define __INET_HTTPBROWSERDIRECT_H

#include "HttpBrowserBase.h"

/**
 * A simulated browser module for OMNeT++ simulations.
 *
 * This module implements direct message passing between modules.
 *
 * @see HttpBrowserBase
 * @see HttpBrowser
 *
 * @author Kristjan V. Jonsson (kristjanvj@gmail.com)
 * @version 1.0
 */
class INET_API HttpBrowserDirect : public HttpBrowserBase
{
    protected:
        /** @name cSimpleModule redefinitions */
        //@{
        /** Initialization of the component and startup of browse event scheduling */
        virtual void initialize(int stage);

        /** Report final statistics */
        virtual void finish();

        /** Handle incoming messages. See the parent class for details. */
        virtual void handleMessage(cMessage *msg);

        /** Returns the number of initialization stages. Two required. */
//      int numInitStages() const {return 2;}
        //@}

    protected:
        /** @name Implementation of methods for sending requests to a server. See parent class for details. */
        //@{
        /** Sends a scripted browse event to a specific server */
        virtual void sendRequestToServer(BrowseEvent be);

        /** Send a request to server. Uses the recipient stamped in the request. */
        virtual void sendRequestToServer(HttpRequestMessage *request);

        /** Sends a generic request to a randomly chosen server */
        virtual void sendRequestToRandomServer();

        /** Sends a number of queued messages to the specified server */
        virtual void sendRequestsToServer(std::string www, HttpRequestQueue queue);
        //@}
};

#endif


