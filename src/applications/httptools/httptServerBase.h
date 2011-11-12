
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
 
#ifndef __httptServerBase_H_
#define __httptServerBase_H_

#include <string>
#include <vector>
#include "httptNodeBase.h"

// Event message kinds
#define MSGKIND_START_SESSION 0
#define MSGKIND_NEXT_MESSAGE  1
#define MSGKIND_SCRIPT_EVENT  2

// Log level definitions
#define LL_NONE 0 
#define LL_INFO 1
#define LL_DEBUG 2

using namespace std;

/**
 * @brief Site definition data structure. Used for scripted sites. 
 */
struct SITE_DEF_STRUCT
{
	long size;
	string body;
};

/**
 * @brief Web server base class 
 *
 * This module implements a flexible Web server. It is part of the HttpTools project
 * and should be used in conjunction with a number of browsing clients.
 *
 * The server base class cannot be instantiated directly in a simulation. Use rather
 * the httptServer for INET TCP/IP appliations or httptServerDirect for direct message passing.
 * See those classes for details. See the INET documentation for details on the StandardHost 
 * and the TCP/IP simulation.
 *
 * @see httptServer
 * @see httptServerDirect
 * @see DirectHost.
 *
 * @version 1.0 
 * @author  Kristjan V. Jonsson
 */
class INET_API httptServerBase : public httptNodeBase
{
  	protected:
		/** set to true if a scripted site definition is used */
		bool scriptedMode;
		/** A map of html pages, keyed by a resource URL. Used in scripted mode. */
		map<string,SITE_DEF_STRUCT> htmlPages;
		/** A map of resource, keyed by a resource URL. Used in scripted mode. */
		map<string,unsigned int> resources;

		// Basic statistics
		long htmlDocsServed;
		long imgResourcesServed;
		long textResourcesServed;
		long badRequests;

		/** @name The random objects for content generation */
		//@{
		rdObject *rdReplyDelay;				//> The processing delay of the server.
		rdObject *rdHtmlPageSize;			//> The HTML page size distribution for the site.
		rdObject *rdTextResourceSize;		//> The text resource size distribution for the site.
		rdObject *rdImageResourceSize;		//> The image resource size distribution for the site.
		rdObject *rdNumResources;			//> Number of resources per HTML page.
		rdObject *rdTextImageResourceRatio; //> The ratio of text resources to images referenced in HTML pages.
		rdObject *rdErrorMsgSize;			//> The size of error messages.
		//@}

		/** The activation time of the server -- initial startup delay. */
		simtime_t activationTime;

	/** @name cSimpleModule redefinitions */
	//@{
	protected:
		/** Initialization of the component and startup of browse event scheduling */
		virtual void initialize();

		/** Report final statistics */
		virtual void finish();

		/** Handle incoming messages */
		virtual void handleMessage(cMessage *msg)=0;
	//@}

	protected:
		void updateDisplay(); //> Update the display string if running in GUI mode

	protected:
		/** Generate a HTML document in response to a request. */
		httptReplyMessage* generateDocument( httptRequestMessage *request, const char* resource, int size=0 );
		/** Generate a resource message in response to a request. */
		httptReplyMessage* generateResourceMessage( httptRequestMessage *request, string resource, CONTENT_TYPE_ENUM category );
		/** Handle a received HTTP GET request */
		httptReplyMessage* handleGetRequest( httptRequestMessage *request, string resource );
		/** Generate a error reply in case of invalid resource requests. */
		httptReplyMessage* generateErrorReply( httptRequestMessage *request, int code );
		/** Create a random body according to the site content random distributions. */
		virtual string generateBody();

	protected:
		/** Handle a received data message, e.g. check if the content requested exists. */
		cMessage* handleReceivedMessage( cMessage *msg );
		/** Register the server object with the controller. Called at initialization (simulation startup). */
		void registerWithController();
		/** Read a site definition from file if a scripted site definition is used. */
		void readSiteDefinition(string file);
		/** Read a html body from a file. Used by readSiteDefinition. */
		string readHtmlBodyFile( string file, string path );
};

#endif


