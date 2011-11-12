
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//
// @file httptMessages.msg
//
// HTTP message definitions for the HttpTools components.
//
// NEW: The module has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @version 1.0 
// @author  Kristjan V. Jonsson
//
// ----------------------------------------------------------------------------

//
// Base class for HTTP messages
//
// NEW: The message definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author Kristjan V. Jonsson
// @version 1.0
//
packet httptBaseMessage
{
    @omitGetVerb(true);
    string targetUrl; 				// The target URL - request set for browsers, echoed by servers.
    string originatorUrl = "";		// The originator URL. Only applicable for servers.
    int protocol = 11; 				// 10 for http/1.0, 11 for http/1.1.
    bool keepAlive = true;			// The keep-alive header
    int serial = 0;					// Convenience field which allows resource requests to be serially tagged for ease of analysis.
    string heading = "";				// The message heading - request string for requests, response for replies
    string payload = "";				// The payload field
}

//
// Message class for HTTP requests
//
// NEW: The message definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author Kristjan V. Jonsson
// @version 1.0
//
packet httptRequestMessage extends httptBaseMessage
{
    @omitGetVerb(true);			// Dont append get to generated getter methods.
    bool badRequest = false;	// Set to true to simulate a bad request. Server answers w. 404.
}

//
// Message class for HTTP replies
//
// NEW: The message definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author Kristjan V. Jonsson
// @version 1.0
//
packet httptReplyMessage extends httptBaseMessage
{
    @omitGetVerb(true);
    int result = 0; 				// e.g. 200 for OK, 404 for NOT FOUND.
    int contentType = 0;			// The content type: HTML pages, text or image resources.
}



