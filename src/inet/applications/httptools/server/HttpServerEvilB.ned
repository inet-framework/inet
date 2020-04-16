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

package inet.applications.httptools.server;

//
// HttpServerEvilB
//
// Demonstrates subclassing the server to create a custom site. This site is an attacker -- a puppetmaster --
// which serves HTML pages containing attack code. In this case, we are simulating JavaScript attack code which prompts
// the unsuspecting browser to issue a number of requests for non-existing resources (random URLS) to the victim site.
// Delays are specified to simulate hiding the attack from the browser user by use of JavaScript timeouts or similar mechanisms.
// The generateBody virtual function is redefined to create a page containing the attack code.
//
// @author  Kristjan V. Jonsson
//
simple HttpServerEvilB extends HttpServer
{
    parameters:
        @class(HttpServerEvilB);
        int minBadRequests;     // Specifies the lower bound of bad requests to be sent to the victim by the browser.
        int maxBadRequests;     // Specifies the upper bound of bad requests to be sent to the victim by the browser.
}

